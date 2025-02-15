#include "DRV8244.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include "dbg_pins.hpp"
#include "faults.hpp"
#include "status.hpp"

#define DEFAULT_NSLEEP 1 // nSleep on by default
#define DEFAULT_DRVOFF 1 // driver off by default
#define DEFAULT_IN1 0    // IN1 off by default
#define DEFAULT_IN2 0    // IN2 off by default
#define DEFAULT_CS 1     // CS high by default

// ! init
// use -1 as driver_id for debug pins
void MotorDriver::init(types::u8 id, types::u16 SPI_SPEED) {
  printf("---> Initializing DRV8244");
  if (id == (types::u8) -1) {
    pinSelector.set_debug_mode(true);
  } else {
    pinSelector.set_driver_id(id);
  }

  printf("Initializing SPI");
  init_spi(SPI_SPEED);

  printf("Initializing pins");
  init_pins();

  printf("---> DRV8244 initialized");
}

void MotorDriver::init_spi(types::u16 SPI_SPEED) {
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

void MotorDriver::init_registers_through_spi() {}

void MotorDriver::init_pins() {
  // Initialize pins
  // nFault
  outputControl.init_digital(pinSelector.get_pin(NFAULT));

  // iPROPi
  inputControl.init(pinSelector.get_pin(IPROPI));
  outputControl.init_digital(pinSelector.get_pin(IPROPI));

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
uint8_t MotorDriver::read8(uint8_t reg) {
  uint8_t txBuf[2];
  uint8_t rxBuf[2];
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

void MotorDriver::write8(uint8_t reg, uint8_t data) {
  uint8_t txBuf[2];
  // If your protocol requires a write bit, you might do:
  // txBuf[0] = reg & 0x7F;
  txBuf[0] = reg;
  txBuf[1] = data;

  // Assert chip-select (active low)
  gpio_put(pinSelector.get_pin(CS), 0);
  spi_write_blocking(spi0, txBuf, 2);
  // Deassert chip-select
  gpio_put(pinSelector.get_pin(CS), 1);
}

//! reading specific registers
std::string MotorDriver::read_fault_summary() {
  uint8_t faultSummary = read8(0x01); // e.g., FAULT_SUMMARY register
  return Fault::get_fault_description(faultSummary);
}

std::string MotorDriver::read_status1() {
  uint8_t statusSummary = read8(0x02); // e.g., FAULT_SUMMARY register
  return Status::get_status1_description(statusSummary);
}

std::string MotorDriver::read_status2() {
  uint8_t statusSummary = read8(0x03); // e.g., FAULT_SUMMARY register
  return Status::get_status1_description(statusSummary);
}

//! on error
void MotorDriver::handle_error(void* _) {
  printf("---> DRV8244 Fault Detected!");

  bool isActive = inputControl.get_last_value(pinSelector.get_pin(NSLEEP));
  if (isActive) {
    printf("Driver is ACTIVE. Reading active state registers...");

    // Read registers that provide diagnostic data during active operation.
    printf("FAULT_SUMMARY: %s", read_fault_summary());
    printf("STATUS1: %s", read_status1());
    printf("STATUS2: %s", read_status1());
  } else {
    printf("Driver is in STANDBY.");
    // TODO: Implement standby state fault handling
  }

  // * try to clear the fault
  printf("Attempting to clear the fault...");
  // TODO: Implement fault clearing
}

void MotorDriver::command(types::u16 duty_cycle, bool direction) {
  bool isActive = inputControl.get_last_value(pinSelector.get_pin(NSLEEP));
  bool isFault = outputControl.read_digital(pinSelector.get_pin(NFAULT));
  bool isOff = inputControl.get_last_value(pinSelector.get_pin(DRVOFF));

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
      printf("Driver has faulted. Cannot command motor.");
      return;
    }
  }

  // * command the motor

}