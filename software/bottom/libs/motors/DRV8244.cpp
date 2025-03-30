#include "debug.hpp"
#include "pinmap.hpp"
#include "registers.hpp"
#include <cstdint>
extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>
}
#include "DRV8244.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include "registers.hpp"
#include "comms.hpp"
#include "pins/digital_pins.hpp"

#define ADC1_ADDR 0b1001000
#define ADC2_ADDR 0b1001001
#define ADC_CLK_SPEED 300 // 0.3 MHz, both fast and high speed modes
#define ADC_DATA_RATE 28

#define DEFAULT_NSLEEP 1 // not sleeping by default
#define DEFAULT_DRVOFF 0 // driver on by default
#define DEFAULT_IN1 0    // IN1 off by default
#define DEFAULT_IN2 0    // IN2 off by default
#define DEFAULT_CS 1     // CS high by default

#define COMMAND_REG 0x08
#define COMMAND_REG_RESET 0b10001001    // CLR FLT, SPI_IN lock, REG unlock
#define COMMAND_REG_EXPECTED 0b00001001 // CLR FLT, SPI_IN lock, REG unlock

#define CONFIG1_REG_RESET 0x10
#define CONFIG1_REG 0x0A

#define CONFIG2_REG_RESET 0x00
#define CONFIG2_REG 0x0B

#define CONFIG3_REG_RESET 0x40
#define CONFIG3_REG 0x0C

#define CONFIG4_REG_RESET 0x04
#define CONFIG4_REG 0x0D

#define STATUS1_REG_EXPECTED 0b00010000
#define STATUS1_REG 0x02

#define STATUS2_REG_EXPECTED 0b00010000
#define STATUS2_REG 0x03

#define FAULT_SUMMARY_REG 0x01

#define SPI_ADDRESS_MASK_WRITE                                                 \
  0x3F00 // Mask for writing SPI register address bits
#define SPI_ADDRESS_MASK_READ                                                  \
  0x7F00                     // Mask for reading SPI register address bits
#define SPI_ADDRESS_POS 8    // Position for SPI register address bits
#define SPI_DATA_MASK 0x00FF // Mask for SPI register data bits
#define SPI_DATA_POS 0       // Position for SPI register data bits
#define SPI_RW_BIT_MASK                                                        \
  0x4000 // Mask for SPI register read write indication bit

namespace driver {
// ! init
// use -1 as driver_id for debug pins
bool MotorDriver::init(int id, spi_inst_t *spi_obj_touse) {
  debug::debug("---> Initializing DRV8244\r\n");
  _id = id;

  if (id == -1) {
    pins.set_debug_mode(true);
  } else {
    pins.set_debug_mode(false);
    pins.set_driver_id(id);
  }

  debug::debug("-> Initializing SPI\r\n");
  spi_obj = spi_obj_touse;
  configure_spi();

  debug::debug("-> Initializing pins\r\n");
  init_pins();

  debug::debug("-> Initializing ADC\r\n");
  if (!adc_init[0] &&
      !adc1.beginADSX((PICO_ADS1115::ADSXAddressI2C_e)ADC1_ADDR, i2c1,
                      ADC_CLK_SPEED, (uint8_t)pinmap::Pico::I2C1_SDA,
                      (uint8_t)pinmap::Pico::I2C1_SCL, 1000)) {
    debug::error("ADC1 not found!\r\n");
    return false;
  } else {
    adc_init[0] = true;
    adc1.setGain(PICO_ADS1115::ADSXGain_TWO);
    adc1.setDataRate(ADC_DATA_RATE);
  }

  if (!adc_init[1] &&
      !adc2.beginADSX((PICO_ADS1115::ADSXAddressI2C_e)ADC2_ADDR, i2c1,
                      ADC_CLK_SPEED, (uint8_t)pinmap::Pico::I2C1_SDA,
                      (uint8_t)pinmap::Pico::I2C1_SCL, 1000)) {
    debug::error("ADC2 not found!\r\n");
    return false;
  } else {
    adc_init[1] = true;
    adc2.setGain(PICO_ADS1115::ADSXGain_TWO);
    adc2.setDataRate(ADC_DATA_RATE);
  }

  debug::debug("-> Configuring registers\r\n");
  if (!init_registers()) {
    debug::error("Error: Could not configure registers\r\n");
    return false;
  }

  debug::debug("---> DRV8244 initialized\r\n");
  return true;
}

void MotorDriver::init_pins() {
  pins::digital_pins.set_mode((pinmap::Digital)pins.get_pin(NFAULT),
                              pins::DigitalPinMode::INPUT_PULLUP);

  pins::digital_pins.set_mode((pinmap::Digital)pins.get_pin(NSLEEP),
                              pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(NSLEEP),
                           DEFAULT_NSLEEP);

  pins::digital_pins.set_mode((pinmap::Digital)pins.get_pin(DRVOFF),
                              pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(DRVOFF),
                           DEFAULT_DRVOFF);

  // Init IN1
  uint8_t pin = pins.get_pin(IN1);
  gpio_set_function(pin, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(pin);
  uint channel = pwm_gpio_to_channel(pin);

  // Set PWM configuration
  pwm_set_clkdiv(slice_num, 4.0);
  pwm_set_wrap(slice_num, 12500); // set max value
  pwm_set_chan_level(slice_num, channel, 0);
  pwm_set_enabled(slice_num, true);

  pins::digital_pins.set_mode((pinmap::Digital)pins.get_pin(IN2),
                              pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(IN2), DEFAULT_IN2);

  pins::digital_pins.set_mode((pinmap::Digital)pins.get_pin(CS),
                              pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(CS), DEFAULT_CS);
}

//! spi/register handling
void MotorDriver::configure_spi() {
  // Initialize SPI pins (except CS)
  gpio_set_function(pins.get_pin(SCK), GPIO_FUNC_SPI);
  gpio_set_function(pins.get_pin(MOSI), GPIO_FUNC_SPI);
  gpio_set_function(pins.get_pin(MISO), GPIO_FUNC_SPI);

  // Set SPI format
  spi_set_format(spi_obj, 16, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
}

bool MotorDriver::write8(uint8_t reg, uint8_t value, int8_t expected) {
  //* prepare data
  uint16_t reg_value = 0;
  uint16_t rx_data = 0;
  reg_value |= ((reg << SPI_ADDRESS_POS) &
                SPI_ADDRESS_MASK_WRITE); // Add register address
  reg_value |= ((value << SPI_DATA_POS) & SPI_DATA_MASK); // Add data value

  //* Write & Read Feedback
  // Initialize CS pin as GPIO

  pins::digital_pins.write((pinmap::Digital)pins.get_pin(CS), 0);
  configure_spi();
  int bytes_written =
      spi_write16_read16_blocking(spi_obj, &reg_value, &rx_data, 1);
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(CS), 1);

  debug::debug("SPI Write - Sent: 0x%04X, Received: 0x%04X\r\n", reg_value,
               rx_data);

  //* Check for no errors in received bytes
  // First 2 MSBs bytes should be '1'
  if ((rx_data & 0xC000) != 0xC000) {
    debug::error(
        "SPI Write - Error: Initial '1' MSB check bytes not found\r\n");
    return false;
  }

  // following 6 bytes are from fault summary
  if ((rx_data & 0x3F00) != 0x0000) {
    debug::error(
        "SPI Write - Error: Fault summary bytes indicating error, %d\r\n",
        rx_data & 0x3F00);
    // get fault register
    types::u8 fault = read8(0x01);

    if (fault == 0) {
      debug::error(
          "SPI Write - No fault found in fault register, moving on...\r\n");
    } else {
      debug::error("SPI Write - %s\n",
                   FAULT::get_fault_description(fault).c_str());
      return false;
    }
  }

  // Check remaining 8 bytes to match the sent data or expected return
  if (expected == -1) {
    if ((rx_data & 0x00FF) != value) {
      debug::error("SPI Write - Error: Data bytes do not match\r\n");
      return false;
    }
  } else {
    if ((rx_data & 0x00FF) != expected) {
      debug::error("SPI Write - Error: Data bytes do not match expected\r\n");
      return false;
    }
  }

  return bytes_written == 1;
}

uint8_t MotorDriver::read8(uint8_t reg) {
  uint16_t reg_value = 0;
  uint16_t rx_data = 0;

  reg_value |= ((reg << SPI_ADDRESS_POS) & SPI_ADDRESS_MASK_READ);
  reg_value |= SPI_RW_BIT_MASK;

  configure_spi();
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(CS), 0);
  spi_write16_read16_blocking(spi_obj, &reg_value, &rx_data, 1);
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(CS), 1);

  debug::warn("SPI Read - Sent: 0x%04X, Received: 0x%04X\r\n", reg_value,
              rx_data);

  //* Check for no errors in received bytes
  // First 2 MSBs bytes should be '1'
  if ((rx_data & 0xC000) != 0xC000) {
    debug::error(
        "SPI Write - Error: Initial '1' MSB check bytes not found\r\n");
    return false;
  }

  // following 6 bytes are from fault summary
  if ((rx_data & 0x3F00) != 0x0000) {
    debug::error(
        "SPI Write - Error: Fault summary bytes indicating error, %d\r\n",
        rx_data & 0x3F00);
    // get fault register
    types::u8 fault = read8(FAULT_SUMMARY_REG);

    if (fault == 0) {
      debug::error(
          "SPI Write - No fault found in fault register, moving on...\r\n");
    } else {
      debug::error("SPI Write - %s\n",
                   FAULT::get_fault_description(fault).c_str());
      return false;
    }
  }

  return rx_data & 0xFF;
}

//! reading specific registers
bool MotorDriver::init_registers() {
  //* COMMAND register
  if (!write8(COMMAND_REG, COMMAND_REG_RESET, COMMAND_REG_EXPECTED)) {
    debug::debug("Error: Could not write to COMMAND register\r\n");
    return false;
  }

  //* CONFIG1 register
  if (!write8(CONFIG1_REG, CONFIG1_REG_RESET)) {
    debug::debug("Error: Could not write to CONFIG1 register\r\n");
    return false;
  }

  //* CONFIG2 register
  if (!write8(CONFIG2_REG, CONFIG2_REG_RESET)) {
    debug::debug("Error: Could not write to CONFIG2 register\r\n");
    return false;
  }

  //* CONFIG3 register
  if (!write8(CONFIG3_REG, CONFIG3_REG_RESET)) {
    debug::debug("Error: Could not write to CONFIG3 register\r\n");
    return false;
  }

  //* CONFIG4 register
  if (!write8(CONFIG4_REG, CONFIG4_REG_RESET)) {
    debug::debug("Error: Could not write to CONFIG4 register\r\n");
    return false;
  }
  return true;
}

bool MotorDriver::check_registers() {
  //* FAULT_SUMMARY register
  types::u8 faultSummary = read8(FAULT_SUMMARY_REG);
  if (faultSummary != 0) {
    debug::debug("Error: FAULT_SUMMARY: %s\r\n",
                 FAULT::get_fault_description(faultSummary).c_str());
    return false;
  }

  //* STATUS1 register
  types::u8 status1 = read8(STATUS1_REG);

  if (status1 != STATUS1_REG_EXPECTED) {
    debug::debug("Error: STATUS1: %s\r\n",
                 STATUS::get_status1_description(status1).c_str());
    return false;
  }

  //* STATUS2 register
  types::u8 status2 = read8(STATUS2_REG);
  if (status2 != STATUS2_REG_EXPECTED) {
    debug::debug("Error: STATUS2: %s\r\n",
                 STATUS::get_status2_description(status2).c_str());
    return false;
  }

  return true;
}

std::string MotorDriver::read_fault_summary() {
  types::u8 faultSummary =
      read8(FAULT_SUMMARY_REG); // e.g., FAULT_SUMMARY register
  return FAULT::get_fault_description(faultSummary);
}

std::string MotorDriver::read_status1() {
  types::u8 statusSummary = read8(0x02); // e.g., FAULT_SUMMARY register
  return STATUS::get_status1_description(statusSummary);
}

std::string MotorDriver::read_status2() {
  types::u8 statusSummary = read8(0x03); // e.g., FAULT_SUMMARY register
  return STATUS::get_status2_description(statusSummary);
}

//! on error
void MotorDriver::handle_error(MotorDriver *driver) {
  debug::error("---> DRV8244 Fault Detected!\r\n");

  // Read registers that provide diagnostic data during active operation.
  std::string fault_summary = "FAULT_SUMMARY: " + driver->read_fault_summary();
  std::string status1 = "STATUS1: " + driver->read_status1();
  std::string status2 = "STATUS2: " + driver->read_status2();
  debug::error("%s\n", fault_summary.c_str());
  debug::error("%s\n", status1.c_str());
  debug::error("%s\n", status2.c_str());

  // * try to clear the fault
  debug::error("Attempting to clear the fault...\n");
  driver->write8(COMMAND_REG, COMMAND_REG_RESET, COMMAND_REG_EXPECTED);

  // * check if the fault was cleared
  if (driver->read8(FAULT_SUMMARY_REG) == 0) {
    debug::info("Fault cleared successfully.\r\n");
  } else {
    debug::error("Fault could not be cleared.\r\n");
    std::string fault_summary =
        "FAULT_SUMMARY: " + driver->read_fault_summary();
    debug::error("%s\n", fault_summary.c_str());
  }
}

bool MotorDriver::check_config() {
  // check if the driver is in fault
  if (!pins::digital_pins.read((pinmap::Digital)pins.get_pin(NFAULT))) {
    debug::warn("Driver has faulted. Cannot command motor.\r\n");

    types::u8 faultSummary = read8(FAULT_SUMMARY_REG);
    debug::warn("Error: FAULT_SUMMARY: %d %s\r\n", faultSummary,
                FAULT::get_fault_description(faultSummary).c_str());
    if (faultSummary != 0) {
      debug::warn("Error: FAULT_SUMMARY: %s\r\n",
                  FAULT::get_fault_description(faultSummary).c_str());
      return false;
    } else {
      debug::warn("Driver is in fault state, but there is no fault... "
                  "clearing fault and continuing\r\n");
      if (!write8(COMMAND_REG, COMMAND_REG_RESET, COMMAND_REG_EXPECTED)) {
        debug::debug("Error: Could not write to COMMAND register\r\n");
        return false;
      }
    }
  }

  // check registers
  if (!check_registers()) {
    debug::error(
        "Driver registers are not configured correctly. Cannot command "
        "motor.\r\n");
    return false;
  }

  return true;
}

void MotorDriver::set_sleep(bool sleep) {
  // Set the sleep pinwrite_analog
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(NSLEEP), !sleep);
  debug::debug("Motor sleep set to %d\r\n", !sleep);
}

int16_t MotorDriver::read_current() {
  // Read the current from the ADC
  if (_id == 1 || _id == 4)
    return adc2.readADC_SingleEnded(
        (PICO_ADS1X15::ADSX_AINX_e)pins.get_pin(IPROPI));
  else
    return adc1.readADC_SingleEnded(
        (PICO_ADS1X15::ADSX_AINX_e)pins.get_pin(IPROPI));
  return 0;
}

bool MotorDriver::command(types::i16 duty_cycle) {
  // Verify if the driver can accept commands
  if (!check_config()) {
    debug::error("Motor command aborted due to configuration error.\r\n");
    return false;
  }
  if (duty_cycle < -12500 || duty_cycle > 12500) {
    debug::error("Invalid duty cycle. Must be between -12500 and 12500.\r\n");
    return false;
  }

  // get direction and speed
  bool direction = duty_cycle < 0;
  duty_cycle = abs(duty_cycle);

  // Command motor by setting one channel to PWM and the other low
  pins::digital_pins.write((pinmap::Digital)pins.get_pin(IN2), direction);

  uint slice_num = pwm_gpio_to_slice_num(pins.get_pin(IN1));
  uint channel = pwm_gpio_to_channel(pins.get_pin(IN1));
  pwm_set_chan_level(slice_num, channel, duty_cycle);

  debug::warn(
      "Motor command executed: IN1: %d Duty cycle = %d, Direction = %d\r\n",
      pins.get_pin(IN1), duty_cycle, direction);
  return true;
}

bool MotorDriver::set_ITRIP(ITRIP::ITRIP current_limit) {
  uint8_t config2_reg = read8(CONFIG2_REG);
  config2_reg |= ((uint8_t)current_limit);
  return write8(CONFIG2_REG, config2_reg);
}

bool MotorDriver::set_OCP(OCP::OCP current_limit) {
  uint8_t config4_reg = read8(CONFIG4_REG);
  config4_reg |= ((uint8_t)current_limit) << 3;
  return write8(CONFIG4_REG, config4_reg);
}
} // namespace driver
