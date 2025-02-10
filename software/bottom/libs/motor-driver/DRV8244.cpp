#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spdlog/spdlog.h"
#include "pinmap.hpp"
#include "pins.hpp"

#define DEFAULT_DRVOFF 1 // driver off by default


class MotorDriver
{
  PinInputControl inputControl;
  PinOutputControl outputControl;

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

  void init_pins()
  {
    // Initialize pins
    // nFault
    outputControl.init_digital(PinMap::NFAULT);

    // iPROPi
    inputControl.init_analog(PinMap::IPROPI, 0);
    outputControl.init_digital(PinMap::IPROPI);

    // nSleep
    inputControl.init_digital(PinMap::NSLEEP, 1);

    // DRVOFF
    inputControl.init_digital(PinMap::DRVOFF, DEFAULT_DRVOFF);

    // EN/IN1
    inputControl.init_digital(PinMap::IN1, 0);

    // PH/IN2
    inputControl.init_digital(PinMap::IN2, 0);
  }

  void init(int32_t SPI_SPEED)
  {
    spdlog::info("---> Initializing DRV8244");

    spdlog::info("Initializing SPI");
    init_spi(SPI_SPEED);

    spdlog::info("Initializing pins");
    init_pins();

    spdlog::info("DRV8244 initialized");
  }
};
