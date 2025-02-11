#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pinmap.hpp"
#include "pins.hpp"
#include "registers.hpp"

#define DEFAULT_NSLEEP 1 // nSleep on by default
#define DEFAULT_DRVOFF 1 // driver off by default
#define DEFAULT_IN1 0    // IN1 off by default
#define DEFAULT_IN2 0    // IN2 off by default

class MotorDriver {
  PinInputControl inputControl;
  PinOutputControl outputControl;
  Registers registers;

  // ! init
  void init_spi(int32_t SPI_SPEED)
  {
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

  void init_registers_through_spi()
  {
  }

  void init_pins()
  {
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

  //! on error
  void on_error(uint gpio, uint32_t events)
  {
    if (events & GPIO_IRQ_EDGE_FALL)
    {
      spdlog::error("---> DRV8244 Fault Detected!");

      bool isActive = this->inputControl.get_last_value(PinMap::NSLEEP);
      if (isActive)
      {
        spdlog::info("Driver is ACTIVE. Reading active state registers...");

        // Read registers that provide diagnostic data during active operation.
        // (Replace register addresses with the correct ones from your datasheet.)
        uint8_t faultSummary = this->registers.readRegister(0x01); // e.g., FAULT_SUMMARY register
        uint8_t status1 = this->registers.readRegister(0x02);      // e.g., STATUS1 register
        uint8_t status2 = this->registers.readRegister(0x03);      // e.g., STATUS2 register

        spdlog::info("FAULT_SUMMARY: 0x{:02X}", faultSummary);
        spdlog::info("STATUS1:       0x{:02X}", status1);
        spdlog::info("STATUS2:       0x{:02X}", status2);
      }
      else
      {
        spdlog::info("Driver is in STANDBY. Reading standby state registers...");

        // In standby you might only be able to read a subset of registers.
        // For example, read the DEVICE_ID register to confirm proper standby operation.
        uint8_t deviceId = this->registers.readRegister(0x00); // e.g., DEVICE_ID register
        spdlog::info("DEVICE_ID: 0x{:02X}", deviceId);
      }

      // * try to clear the fault
      spdlog::info("Attempting to clear the fault...");
    }

    if (events & GPIO_IRQ_EDGE_RISE)
    {
      spdlog::info("---> DRV8244 Fault Cleared.");
    }
  }

public:
  void init(int32_t SPI_SPEED)
  {
    spdlog::info("---> Initializing DRV8244");

    spdlog::info("Initializing SPI");
    init_spi(SPI_SPEED);

    spdlog::info("Initializing pins");
    init_pins();

    spdlog::info("Listen for errors");
    gpio_set_irq_enabled_with_callback(PinMap::NFAULT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, );

    spdlog::info("---> DRV8244 initialized");
  }

  void command(uint32_t speed, uint32_t direction) {}
};
