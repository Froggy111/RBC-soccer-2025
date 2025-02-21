#include <cstdint>
extern "C" {
#include <hardware/spi.h>
#include <hardware/pwm.h>
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
#define COMMAND_REG_DEFAULT 0b10001001 // CLR FLT, SPI_IN lock, REG unlock

#define CONFIG1_REG_RESET 0x10
#define CONFIG1_REG 0x0A

#define CONFIG2_REG_RESET 0x00
#define CONFIG2_REG 0x0B

#define CONFIG3_REG_RESET 0x40
#define CONFIG3_REG 0x0C
#define CONFIG3_REG_DEFAULT 0b01000011

#define CONFIG4_REG_RESET 0x04
#define CONFIG4_REG 0x0D

// ! init
// use -1 as driver_id for debug pins
void MotorDriver::init(types::u8 id, types::u64 SPI_SPEED) {
  printf("---> Initializing DRV8244\n");
  if (id == (types::u8)-1) {
    pinSelector.set_debug_mode(true);
  } else {
    pinSelector.set_driver_id(id);
  }

  printf("Initializing SPI\n");
  init_spi(SPI_SPEED);

  printf("Initializing pins\n");
  init_pins();

  printf("Configuring registers\n");
  init_registers();

  printf("---> DRV8244 initialized\n");
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
  inputControl.init_digital(pinSelector.get_pin(IN1), DEFAULT_IN1);

  // PH/IN2
  inputControl.init_digital(pinSelector.get_pin(IN2), DEFAULT_IN2);
}

//! register handling
types::u8 MotorDriver::read8(types::u8 reg_addr) {
  // TODO: FIX THIS
  uint8_t tx[2];
  uint16_t rx[1];

  // Set MSB to 1 to indicate a read operation.
  tx[0] = 0b10000000 | reg_addr;
  tx[1] = 0b00000000; // Dummy byte

  inputControl.write_digital(pinSelector.get_pin(CS),
                             0); // Activate chip select
  spi_read16_blocking(spi0, *tx, rx, 2);
  inputControl.write_digital(pinSelector.get_pin(CS),
                             1); // Activate chip select

  // rx[1] contains the register value read back.
  return rx[1];
}

bool MotorDriver::write8(types::u8 reg, types::u8 data, types::u8 mask) {
  // Read current register value to apply mask.
  types::u8 current = read8(reg);
  types::u8 new_value = data; // TODO: (current & ~mask) | (data & mask);

  types::u8 tx[2];

  // Set MSB to 0 to indicate a write operation.
  tx[0] = reg & 0b01111111;
  tx[1] = new_value;

  inputControl.write_digital(pinSelector.get_pin(CS),
                             0); // Activate chip select
  printf("Sent SPI data frame: ");
  for (int i = 7; i >= 0; i--) {
      printf("%d", (tx[0] >> i) & 1);
  }
  for (int i = 7; i >= 0; i--) {
      printf("%d", (tx[1] >> i) & 1);
  }
  printf("\n");

  spi_write_blocking(spi0, tx, 2);
  inputControl.write_digital(pinSelector.get_pin(CS),
                             1); // Activate chip select

  // read the spi data frame sent back
  tx[0] = 0b10000000 | reg;
  tx[1] = 0b00000000;
  uint16_t rx[1];
  spi_read16_blocking(spi0, *tx, rx, 2);

  printf("SPI data frame sent back: ");
  for (int i = 15; i >= 0; i--) {
      printf("%d", (rx[0] >> i) & 1);
  }
  printf("\n");
  return true;
}

//! reading specific registers
void MotorDriver::init_registers() {
  //* COMMAND register
  if (!write8(COMMAND_REG, COMMAND_REG_DEFAULT)) {
    printf("Error: Could not write to COMMAND register\n");
  }

  //* CONFIG1 register
  if (!write8(CONFIG1_REG, CONFIG1_REG_RESET)) {
    printf("Error: Could not write to CONFIG1 register\n");
  }

  //* CONFIG2 register
  if (!write8(CONFIG2_REG, CONFIG2_REG_RESET)) {
    printf("Error: Could not write to CONFIG2 register\n");
  }

  //* CONFIG3 register
  if (!write8(CONFIG3_REG, CONFIG3_REG_DEFAULT)) {
    printf("Error: Could not write to CONFIG3 register\n");
  }

  //* CONFIG4 register
  if (write8(CONFIG4_REG, CONFIG4_REG_RESET)) {
    printf("Error: Could not write to CONFIG4 register\n");
  }
}

bool MotorDriver::check_registers() {
  //TODO: implement register checking

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
  printf("---> DRV8244 Fault Detected!\n");

  // Read registers that provide diagnostic data during active operation.
  std::string fault_summary = "FAULT_SUMMARY: " + driver->read_fault_summary();
  std::string status1 = "STATUS1: " + driver->read_status1();
  std::string status2 = "STATUS2: " + driver->read_status2();
  printf("%s\n", fault_summary.c_str());
  printf("%s\n", status1.c_str());
  printf("%s\n", status2.c_str());

  // * try to clear the fault
  printf("Attempting to clear the fault...\n");
  driver->write8(0x08, 0b0000001, 0b0000001);

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
  if (!inputControl.get_last_value(pinSelector.get_pin(NSLEEP))) {
    printf("Driver is not active. Cannot command motor.\n");
    return false;
  }

  // check if the driver is off
  if (inputControl.get_last_value(pinSelector.get_pin(DRVOFF))) {
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
  return true;
}

bool MotorDriver::command(types::u16 duty_cycle, bool direction) {
  // Verify if the driver can accept commands
  if (!check_config()) {
    printf("Motor command aborted due to configuration error.\n");
    return false;
  }

  // Retrieve the physical pin numbers for motor control
  uint in1_pin = pinSelector.get_pin(IN1);
  uint in2_pin = pinSelector.get_pin(IN2);

  // Command motor by setting one channel to PWM and the other low
  // gpio_put(in2_pin, direction);
  // pwm_set_gpio_level(in1_pin, duty_cycle);
  // std::string debug =
  //     "Motor command executed: Duty cycle = " + std::to_string(duty_cycle) +
  //     ", Direction = " + std::to_string(direction);
  // printf("%s\n", debug.c_str());
  return true;
}
