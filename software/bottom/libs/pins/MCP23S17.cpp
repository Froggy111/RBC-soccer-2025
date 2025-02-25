#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"

extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <stdio.h>
}

#define ADDRESS_1 0b111
#define ADDRESS_2 0b000
#define DEFAULT_CS 1     // CS high by default

void MCP23S17::init(types::u8 device_id, types::u32 baudrate) {
  id = device_id;

  init_spi(baudrate);
}

void MCP23S17::init_spi(types::u32 baudrate) {
  // Initialize SPI with error checking
  if (!spi_init(spi0, baudrate)) {
    printf("Error: SPI initialization failed\n");
    return;
  }

  // Initialize SPI pins (except CS)
  gpio_set_function((uint) pinmap::DigitalPins::SPI0_SCLK, GPIO_FUNC_SPI);
  gpio_set_function((uint) pinmap::DigitalPins::SPI0_MOSI, GPIO_FUNC_SPI);
  gpio_set_function((uint) pinmap::DigitalPins::SPI0_MISO, GPIO_FUNC_SPI);

  // Initialize CS pin as GPIO
  gpio_init((uint) pinmap::DigitalPins::DMUX_SCS);
  gpio_set_dir((uint) pinmap::DigitalPins::DMUX_SCS, GPIO_OUT);
  gpio_put((uint) pinmap::DigitalPins::DMUX_SCS, DEFAULT_CS);

  // Set SPI format
  spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
}

void MCP23S17::write8(uint8_t reg_address, uint8_t data){
    gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 0);

    gpio_put((uint) pinmap::DigitalPins::DMUX_SCS, 1);
}

uint8_t MCP23S17::read8(uint8_t reg_address) {
  gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 0);

  gpio_put((uint) pinmap::DigitalPins::DMUX_SCS, 1);
}