#include <cstdint>
extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <stdio.h>
}
#include "DRV8244.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include "dbg_pins.hpp"
#include "faults.hpp"
#include "status.hpp"

#define DEFAULT_NSLEEP 1 // sleep by default
#define DEFAULT_DRVOFF 0 // driver on by default
#define DEFAULT_IN1 0    // IN1 off by default
#define DEFAULT_IN2 0    // IN2 off by default
#define DEFAULT_CS 1     // CS high by default

#define COMMAND_REG 0x08
#define COMMAND_REG_RESET 0b10001001 // CLR FLT, SPI_IN lock, REG unlock
#define COMMAND_REG_EXPECTED 0b00001001 // CLR FLT, SPI_IN lock, REG unlock

#define CONFIG1_REG_RESET 0x10
#define CONFIG1_REG 0x0A

#define CONFIG2_REG_RESET 0x00
#define CONFIG2_REG 0x0B

#define CONFIG3_REG_RESET 0x40
#define CONFIG3_REG 0x0C

#define CONFIG4_REG_RESET 0x04
#define CONFIG4_REG 0x0D

#define STATUS1_REG_EXPECTED 0b00001000
#define STATUS1_REG 0x02

#define STATUS2_REG_EXPECTED 0b10001000
#define STATUS2_REG 0x03

#define FAULT_SUMMARY_REG 0x01

#define SPI_ADDRESS_MASK_WRITE 0x3F00 // Mask for writing SPI register address bits
#define SPI_ADDRESS_MASK_READ 0x7F00                     // Mask for reading SPI register address bits
#define SPI_ADDRESS_POS 8    // Position for SPI register address bits
#define SPI_DATA_MASK 0x00FF // Mask for SPI register data bits
#define SPI_DATA_POS 0       // Position for SPI register data bits
#define SPI_RW_BIT_MASK 0x4000 // Mask for SPI register read write indication bit

// ! init
// use -1 as driver_id for debug pins
void MotorDriver::init(int id, types::u64 SPI_SPEED) {
  printf("---> Initializing DRV8244\n\n");
  if (id == -1) {
    pinSelector.set_debug_mode(true);
  } else {
    pinSelector.set_debug_mode(false);
    pinSelector.set_driver_id(id);
  }

  printf("-> Initializing SPI\n");
  init_spi(SPI_SPEED);

  printf("-> Initializing pins\n");
  init_pins();

  printf("-> Configuring registers\n");
  if (!init_registers()) {
    printf("Error: Could not configure registers\n");
    return;
  }

  printf("---> DRV8244 initialized\n\n");
}

void MotorDriver::init_spi(types::u64 SPI_SPEED) {
  // Initialize SPI with error checking
  if (!spi_init(spi0, SPI_SPEED)) {
    printf("Error: SPI initialization failed\n");
    return;
  }

  // Initialize SPI pins (except CS)
  gpio_set_function(pinSelector.get_pin(SCK), GPIO_FUNC_SPI);
  gpio_set_function(pinSelector.get_pin(MOSI), GPIO_FUNC_SPI);
  gpio_set_function(pinSelector.get_pin(MISO), GPIO_FUNC_SPI);

  // Initialize CS pin as GPIO
  inputControl.init_digital(pinSelector.get_pin(CS), DEFAULT_CS);

  // Set SPI format
  spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
}

void MotorDriver::init_pins() {
  // Initialize pins
  // nFault
  outputControl.init_digital(pinSelector.get_pin(NFAULT));

  // nSleep
  inputControl.init_digital(pinSelector.get_pin(NSLEEP), DEFAULT_NSLEEP);

  // DRVOFF
  inputControl.init_digital(pinSelector.get_pin(DRVOFF), DEFAULT_DRVOFF);

  // EN/IN1
  inputControl.init_analog(pinSelector.get_pin(IN1), DEFAULT_IN1);

  // PH/IN2
  inputControl.init_digital(pinSelector.get_pin(IN2), DEFAULT_IN2);
}

//! register handling
bool MotorDriver::write8(uint8_t reg, uint8_t value, int8_t expected) {
  //* prepare data
  uint16_t reg_value = 0;
  uint16_t rx_data = 0;
  reg_value |= ((reg << SPI_ADDRESS_POS) &
                SPI_ADDRESS_MASK_WRITE); // Add register address
  reg_value |= ((value << SPI_DATA_POS) & SPI_DATA_MASK); // Add data value

  //* Write & Read Feedback
  inputControl.write_digital(pinSelector.get_pin(CS), 0);
  int bytes_written =
      spi_write16_read16_blocking(spi0, &reg_value, &rx_data, 1);
  inputControl.write_digital(pinSelector.get_pin(CS), 1);

  printf("SPI Write - Sent: 0x%04X, Received: 0x%04X\n", reg_value, rx_data);

  //* Check for no errors in received bytes
  // First 2 MSBs bytes should be '1'
  if ((rx_data & 0xC000) != 0xC000) {
    printf("SPI Write - Error: Initial '1' MSB check bytes not found\n");
    return false;
  }

  // following 6 bytes are from fault summary
  if ((rx_data & 0x3F00) != 0x0000) {
    printf("SPI Write - Error: Fault summary bytes indicating error, %d\n",
           rx_data & 0x3F00);
    // get fault register
    types::u8 fault = read8(0x01);

    if (fault == 0) {
      printf("SPI Write - No fault found in fault register, moving on...\n");
    } else {
      printf("SPI Write - %s\n", Fault::get_fault_description(fault).c_str());
      return false;
    }
  }

  // Check remaining 8 bytes to match the sent data or expected return
  if (expected == -1) {
    if ((rx_data & 0x00FF) != value) {
      printf("SPI Write - Error: Data bytes do not match\n");
      return false;
    }
  } else {
    if ((rx_data & 0x00FF) != expected) {
      printf("SPI Write - Error: Data bytes do not match expected\n");
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

  inputControl.write_digital(pinSelector.get_pin(CS), 0);
  spi_write16_read16_blocking(spi0, &reg_value, &rx_data, 1);
  inputControl.write_digital(pinSelector.get_pin(CS), 1);

  printf("SPI Read - Sent: 0x%04X, Received: 0x%04X\n", reg_value, rx_data);

  //* Check for no errors in received bytes
  // First 2 MSBs bytes should be '1'
  if ((rx_data & 0xC000) != 0xC000) {
    printf("SPI Write - Error: Initial '1' MSB check bytes not found\n");
    return false;
  }

  // following 6 bytes are from fault summary
  if ((rx_data & 0x3F00) != 0x0000) {
    printf("SPI Write - Error: Fault summary bytes indicating error, %d\n",
           rx_data & 0x3F00);
    // get fault register
    types::u8 fault = read8(FAULT_SUMMARY_REG);

    if (fault == 0) {
      printf("SPI Write - No fault found in fault register, moving on...\n");
    } else {
      printf("SPI Write - %s\n", Fault::get_fault_description(fault).c_str());
      return false;
    }
  }
  
  return rx_data & 0xFF;
}

//! reading specific registers
bool MotorDriver::init_registers() {
  //* COMMAND register
  if (!write8(COMMAND_REG, COMMAND_REG_RESET, COMMAND_REG_EXPECTED)) {
    printf("Error: Could not write to COMMAND register\n");
    return false;
  }

  //* CONFIG1 register
  if (!write8(CONFIG1_REG, CONFIG1_REG_RESET)) {
    printf("Error: Could not write to CONFIG1 register\n");
    return false;
  }

  //* CONFIG2 register
  if (!write8(CONFIG2_REG, CONFIG2_REG_RESET)) {
    printf("Error: Could not write to CONFIG2 register\n");
    return false;
  }

  //* CONFIG3 register
  if (!write8(CONFIG3_REG, CONFIG3_REG_RESET)) {
    printf("Error: Could not write to CONFIG3 register\n");
    return false;
  }

  //* CONFIG4 register
  if (!write8(CONFIG4_REG, CONFIG4_REG_RESET)) {
    printf("Error: Could not write to CONFIG4 register\n");
    return false;
  }
  return true;
}

bool MotorDriver::check_registers() {
  //* FAULT_SUMMARY register
  types::u8 faultSummary = read8(FAULT_S);
  if (faultSummary != 0) {
    printf("Error: FAULT_SUMMARY: %s\n",
           Fault::get_fault_description(faultSummary).c_str());
    return false;
  }

  //* STATUS1 register
  types::u8 status1 = read8(STATUS1_REG);
  if (status1 != STATUS1_REG_EXPECTED) {
    printf("Error: STATUS1: %s\n",
           Status::get_status1_description(status1).c_str());
    return false;
  }

  //* STATUS2 register
  types::u8 status2 = read8(STATUS2_REG);
  if (status2 != STATUS2_REG_EXPECTED) {
    printf("Error: STATUS2: %s\n",
           Status::get_status2_description(status2).c_str());
    return false;
  }

  return true;
}

std::string MotorDriver::read_fault_summary() {
  types::u8 faultSummary = read8(FAULT_SUMMARY_REG); // e.g., FAULT_SUMMARY register
  return Fault::get_fault_description(faultSummary);
}

std::string MotorDriver::read_status1() {
  types::u8 statusSummary = read8(0x02); // e.g., FAULT_SUMMARY register
  return Status::get_status1_description(statusSummary);
}

std::string MotorDriver::read_status2() {
  types::u8 statusSummary = read8(0x03); // e.g., FAULT_SUMMARY register
  return Status::get_status1_description(statusSummary);
}

//! on error
void MotorDriver::handle_error(MotorDriver *driver) {
  printf("---> DRV8244 Fault Detected!\n\n");

  // Read registers that provide diagnostic data during active operation.
  std::string fault_summary = "FAULT_SUMMARY: " + driver->read_fault_summary();
  std::string status1 = "STATUS1: " + driver->read_status1();
  std::string status2 = "STATUS2: " + driver->read_status2();
  printf("%s\n", fault_summary.c_str());
  printf("%s\n", status1.c_str());
  printf("%s\n", status2.c_str());

  // * try to clear the fault
  printf("Attempting to clear the fault...\n");
  // driver->write8(0x08, 0b0000001, 0b0000001); // TODO

  // * check if the fault was cleared
  if (driver->read8(0x01) == 0) {
    printf("Fault cleared successfully.\n");
  } else {
    printf("Fault could not be cleared.\n");
    std::string fault_summary =
        "FAULT_SUMMARY: " + driver->read_fault_summary();
    printf("%s\n", fault_summary.c_str());
  }
}

bool MotorDriver::check_config() {
  // check if the driver is active
  if (!inputControl.get_last_value_digital(pinSelector.get_pin(NSLEEP))) {
    printf("Driver is not active. Cannot command motor.\n");
    return false;
  }

  // check if the driver is off
  if (inputControl.get_last_value_digital(pinSelector.get_pin(DRVOFF))) {
    printf("Driver is off. Cannot command motor.\n");
    return false;
  }

  // check if the driver is in fault
  if (outputControl.read_digital(pinSelector.get_pin(NFAULT))) {
    printf("Driver has faulted. Cannot command motor.\n");
    return false;
  }

  // check registers
  if (!check_registers()) {
    printf("Driver registers are not configured correctly. Cannot command "
           "motor.\n");
    return false;
  }
  printf("Config OK\n");
  return true;
}

// negative if backwards, positive if forwards
bool MotorDriver::command(types::i16 duty_cycle) {
  // Verify if the driver can accept commands
  if (!check_config()) {
    printf("Motor command aborted due to configuration error.\n");
    return false;
  }

  // Retrieve the physical pin numbers for motor control
  uint in1_pin = pinSelector.get_pin(IN1);
  uint in2_pin = pinSelector.get_pin(IN2);

  // get direction and speed
  bool direction = duty_cycle < 0;
  duty_cycle = abs(duty_cycle);

  // Command motor by setting one channel to PWM and the other low
  inputControl.write_digital(in2_pin, direction);
  inputControl.write_analog(in1_pin, duty_cycle);

  std::string debug =
      "Motor command executed: Duty cycle = " + std::to_string(duty_cycle) +
      ", Direction = " + std::to_string(direction);
  printf("%s\n", debug.c_str());
  return true;
}
