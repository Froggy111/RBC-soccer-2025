#include "DRV8244.hpp"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "libs/utils/types.hpp"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pinmap.hpp"
#include "pins.hpp"
#include "faults.hpp"

#define DEFAULT_NSLEEP 1 // nSleep on by default
#define DEFAULT_DRVOFF 1 // driver off by default
#define DEFAULT_IN1 0    // IN1 off by default
#define DEFAULT_IN2 0    // IN2 off by default

// ! init
void MotorDriver::init_spi(types::u32 SPI_SPEED) {
  // Initialize SPI pins
  gpio_set_function(PinMap::SCK, GPIO_FUNC_SPI);
  gpio_set_function(PinMap::MOSI, GPIO_FUNC_SPI);
  gpio_set_function(PinMap::MISO, GPIO_FUNC_SPI);
  gpio_set_function(PinMap::CS, GPIO_FUNC_SPI);

  // Set SPI format
  spi_init(spi0, SPI_SPEED);
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

  // Set CS pin as output
  gpio_init(PinMap::CS);
  gpio_set_dir(PinMap::CS, GPIO_OUT);
  gpio_put(PinMap::CS, 1); // Set CS high (inactive)
}

void MotorDriver::init_registers_through_spi() {}

void MotorDriver::init_pins() {
  // Initialize pins
  // nFault
  outputControl.init_digital(PinMap::NFAULT);

  // iPROPi
  inputControl.init(PinMap::IPROPI);
  outputControl.init_digital(PinMap::IPROPI);

  // nSleep
  inputControl.init_digital(PinMap::NSLEEP, DEFAULT_NSLEEP);

  // DRVOFF
  inputControl.init_digital(PinMap::DRVOFF, DEFAULT_DRVOFF);

  // EN/IN1
  inputControl.init_digital(PinMap::IN1, DEFAULT_IN1);

  // PH/IN2
  inputControl.init_digital(PinMap::IN2, DEFAULT_IN2);
}

void MotorDriver::init(types::u8 id, types::u16 SPI_SPEED) {
  printf("---> Initializing DRV8244");

  printf("Initializing SPI");
  init_spi(SPI_SPEED);

  printf("Initializing pins");
  init_pins();

  printf("---> DRV8244 initialized");
}

//! register handling
uint8_t MotorDriver::read8(uint8_t reg) {
  uint8_t txBuf[2];
  uint8_t rxBuf[2];
  // If your protocol requires a read bit, you might do:
  // txBuf[0] = reg | 0x80;
  txBuf[0] = reg;
  txBuf[1] = 0x00; // dummy byte

  // Assert chip-select (active low)
  gpio_put(PinMap::CS, 0);
  spi_write_read_blocking(spi0, txBuf, rxBuf, 2);
  // Deassert chip-select
  gpio_put(PinMap::CS, 1);

  // Return the second byte received (the register data)
  return rxBuf[1];
}

void MotorDriver::write8(uint8_t reg, uint8_t data) {
  uint8_t txBuf[2];
  // If your protocol requires a write bit, you might do:
  // txBuf[0] = reg & 0x7F;
  txBuf[0] = reg;
  txBuf[1] = data;

  // Assert chip-select (active low)
  gpio_put(PinMap::CS, 0);
  spi_write_blocking(spi0, txBuf, 2);
  // Deassert chip-select
  gpio_put(PinMap::CS, 1);
}

//! on error
void MotorDriver::handle_error() {
  printf("---> DRV8244 Fault Detected!");

  bool isActive = inputControl.get_last_value(PinMap::NSLEEP);
  if (isActive) {
    printf("Driver is ACTIVE. Reading active state registers...");

    // Read registers that provide diagnostic data during active operation.
    // (Replace register addresses with the correct ones from your datasheet.)
    uint8_t faultSummary = read8(0x01); // e.g., FAULT_SUMMARY register
    uint8_t status1 = read8(0x02);      // e.g., STATUS1 register
    uint8_t status2 = read8(0x03);      // e.g., STATUS2 register

    printf("FAULT_SUMMARY: %s", Fault::get_fault_description(faultSummary));
    printf("STATUS1:       0x{%d}", status1);
    printf("STATUS2:       0x{%d}", status2);
  } else {
    printf("Driver is in STANDBY.");
    // TODO: Implement standby state fault handling
  }

  // * try to clear the fault
  printf("Attempting to clear the fault...");
  // TODO: Implement fault clearing
}

void MotorDriver::command(types::u16 duty_cycle, bool direction) {
  bool isActive = inputControl.get_last_value(PinMap::NSLEEP);
  bool isFault = outputControl.read_digital(PinMap::NFAULT);
  bool isOff = inputControl.get_last_value(PinMap::DRVOFF);

  // trying to optimize for overhead when conditions are correct
  // but compiler exists
  if (!isActive || isFault || isOff) {
    // * check if the driver is active
    if (!isActive) {
      printf("Driver is not active. Cannot command motor.");
      return;
    }

    // * check if the driver is off
    if (isOff) {
      printf("Driver is off. Cannot command motor.");
      return;
    }

    // * check if the driver is in fault
    if (isFault) {
      printf("Driver is in fault. Attempting to clear fault...");
      handle_error();
      return;
    }
  }

  // * command the motor

}