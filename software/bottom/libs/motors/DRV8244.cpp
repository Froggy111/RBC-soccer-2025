#include "DRV8244.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include "dbg_pins.hpp"
#include "faults.hpp"
#include "status.hpp"
#include "debug.hpp"
#include "hardware/spi.h"
#include "hardware/pwm.h"

#define DEFAULT_NSLEEP 0 // sleep by default
#define DEFAULT_DRVOFF 0 // driver on by default
#define DEFAULT_IN1 0    // IN1 off by default
#define DEFAULT_IN2 0    // IN2 off by default
#define DEFAULT_CS 1     // CS high by default

#define COMMAND_REG_DEFAULT 0b00000000
#define CONFIG3_REG_DEFAULT 0b00000000
#define CONFIG3_REG_MASK 0b11000000


// ! init
// use -1 as driver_id for debug pins
void MotorDriver::init(types::u8 id, types::u64 SPI_SPEED) {
  debug::msg("---> Initializing DRV8244");
  if (id == (types::u8)-1) {
    pinSelector.set_debug_mode(true);
  } else {
    pinSelector.set_driver_id(id);
  }

  debug::msg("Initializing SPI");
  init_spi(SPI_SPEED);

  debug::msg("Initializing pins");
  init_pins();

  debug::msg("Configuring registers");
  config_registers();

  debug::msg("---> DRV8244 initialized");
}

void MotorDriver::init_spi(types::u64 SPI_SPEED) {
  // Initialize SPI pins
  gpio_set_function(pinSelector.get_pin(SCK), GPIO_FUNC_SPI);
  gpio_set_function(pinSelector.get_pin(MOSI), GPIO_FUNC_SPI);
  gpio_set_function(pinSelector.get_pin(MISO), GPIO_FUNC_SPI);
  gpio_set_function(pinSelector.get_pin(CS), GPIO_FUNC_SPI);

  // Set SPI format
  spi_init(spi0, SPI_SPEED);
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  // Set CS pin as output
  gpio_init(pinSelector.get_pin(CS));
  gpio_set_dir(pinSelector.get_pin(CS), GPIO_OUT);
  gpio_put(pinSelector.get_pin(CS), DEFAULT_CS);
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
  inputControl.init_digital(pinSelector.get_pin(IN1), DEFAULT_IN1);

  // PH/IN2
  inputControl.init_digital(pinSelector.get_pin(IN2), DEFAULT_IN2);
}

//! register handling
types::u8 MotorDriver::read8(types::u8 reg) {
  types::u8 txBuf[2];
  types::u8 rxBuf[2];
  // If your protocol requires a read bit, you might do:
  // txBuf[0] = reg | 0x80;
  txBuf[0] = reg;
  txBuf[1] = 0x00; // dummy byte

  // Assert chip-select (active low)
  gpio_put(pinSelector.get_pin(CS), 0);
  spi_write_read_blocking(spi0, txBuf, rxBuf, 2);
  // Deassert chip-select
  gpio_put(pinSelector.get_pin(CS), 1);

  // Return the second byte received (the register data)
  return rxBuf[1];
}

void MotorDriver::write8(types::u8 reg, types::u8 data, types::u8 mask) {
  // Read current value if we're doing a partial write
  types::u8 current_value = (mask == 0xFF) ? 0 : read8(reg);
  
  // Clear the bits we want to write (using inverted mask)
  current_value &= ~mask;
  
  // Set the new bits (masked to only affect the bits we want to change)
  types::u8 new_value = current_value | (data & mask);

  types::u8 txBuf[2];
  txBuf[0] = reg;
  txBuf[1] = new_value;

  // Assert chip-select (active low)
  gpio_put(pinSelector.get_pin(CS), 0);
  spi_write_blocking(spi0, txBuf, 2);
  // Deassert chip-select
  gpio_put(pinSelector.get_pin(CS), 1);
}

//! reading specific registers
void MotorDriver::config_registers() {
  //* COMMAND Register
  // reset CLR_FLT, lock SPI_IN, unlock CONFIG registers
  write8(0x00, COMMAND_REG_DEFAULT);

  //* CONFIG3 Register
  // change S_MODE to PH/EN
  write8(0x0C, CONFIG3_REG_DEFAULT, CONFIG3_REG_MASK);  // Only modify the first two bits (bits 7-6)
}

bool MotorDriver::check_registers() {
  // Check the CONFIG3 register to ensure S_MODE is set to PH/EN
  types::u8 config3 = read8(0x0C);
  if ((config3 & CONFIG3_REG_MASK) != CONFIG3_REG_DEFAULT) {
    debug::msg("CONFIG3 register is not set to PH/EN mode.");
    return false;
  }

  return true;
}

std::string MotorDriver::read_fault_summary() {
  types::u8 faultSummary = read8(0x01); // e.g., FAULT_SUMMARY register
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
  debug::msg("---> DRV8244 Fault Detected!");

  // Read registers that provide diagnostic data during active operation.
  debug::msg("FAULT_SUMMARY: " + driver->read_fault_summary());
  debug::msg("STATUS1: " + driver->read_status1());
  debug::msg("STATUS2: " + driver->read_status1());

  // * try to clear the fault
  debug::msg("Attempting to clear the fault...");
  driver->write8(0x08, 0b0000001, 0b0000001);

  // * check if the fault was cleared
  if (driver->read8(0x01) == 0) {
    debug::msg("Fault cleared successfully.");
  } else {
    debug::msg("Fault could not be cleared.");
    debug::msg("FAULT_SUMMARY: " + driver->read_fault_summary());
  }
}

bool MotorDriver::check_config() {
  // check if the driver is active
  if (!inputControl.get_last_value(pinSelector.get_pin(NSLEEP))) {
    debug::msg("Driver is not active. Cannot command motor.");
    return false;
  }

  // check if the driver is off
  if (inputControl.get_last_value(pinSelector.get_pin(DRVOFF))) {
    debug::msg("Driver is off. Cannot command motor.");
    return false;
  }

  // check if the driver is in fault
  if (outputControl.read_digital(pinSelector.get_pin(NFAULT))) {
    debug::msg("Driver has faulted. Cannot command motor.");
    return false;
  }

  // check registers
  if (!check_registers()) {
    debug::msg("Driver registers are not configured correctly. Cannot command motor.");
    return false;
  }
}

bool MotorDriver::command(types::u16 duty_cycle, bool direction) {
  // Verify if the driver can accept commands
  if (!check_config()) {
    debug::msg("Motor command aborted due to configuration error.");
    return false;
  }

  // Retrieve the physical pin numbers for motor control
  uint in1_pin = pinSelector.get_pin(IN1);
  uint in2_pin = pinSelector.get_pin(IN2);

  // Command motor by setting one channel to PWM and the other low
  if (direction) {
    // For one direction, apply PWM on IN1 and drive IN2 low
    gpio_put(in2_pin, 0);
    pwm_set_gpio_level(in1_pin, duty_cycle);
  } else {
    // For the reverse direction, apply PWM on IN2 and drive IN1 low
    gpio_put(in1_pin, 0);
    pwm_set_gpio_level(in2_pin, duty_cycle);
  }

  debug::msg(
      "Motor command executed: Duty cycle = " + std::to_string(duty_cycle) +
      ", Direction = " + std::to_string(direction));
  return true;
}