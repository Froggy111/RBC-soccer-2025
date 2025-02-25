#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include <pico/time.h>

extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <stdio.h>
}

// addresses of the MCP23S17
#define ADDRESS_1 0b000
#define ADDRESS_2 0b001

// default values for pins
#define DEFAULT_CS 1 // CS high by default

// register addresses
#define IODIRA 0x00
#define IPOLA 0x01
#define GPINTENA 0x02
#define DEFVALA 0x03
#define INTCONA 0x04
#define IOCONA 0x05
#define GPPUA 0x06
#define INTFA 0x07
#define INTCAPA 0x08
#define GPIOA 0x09
#define OLATA 0x0A

#define IODIRB 0x10
#define IPOLB 0x11
#define GPINTENB 0x12
#define DEFVALB 0x13
#define INTCONB 0x14
#define IOCONB 0x15
#define GPPUB 0x16
#define INTFB 0x17
#define INTCAPB 0x18
#define GPIOB 0x19
#define OLATB 0x1A

void MCP23S17::init(types::u8 device_id, types::u32 baudrate) {
  printf("---> Initializing MCP23S17\n\n");
  if (device_id != 1 && device_id != 2) {
    printf("Error: Invalid device ID\n");
    return;
  }
  id = device_id;

  // init gpio pins
  printf("-> Initializing pins\n");
  init_pins();

  // init spi
  printf("-> Initializing SPI\n");
  init_spi(baudrate);
}

void MCP23S17::init_pins() {
  // Initialize RESET
  gpio_init((uint) pinmap::DigitalPins::DMUX_RESET);
  gpio_set_dir((uint)pinmap::DigitalPins::DMUX_RESET, GPIO_OUT);
  reset();

  // initialize INTA
  if (id == 2) {
    gpio_init((uint) pinmap::DigitalPins::DMUX2_INT);
    gpio_set_dir((uint) pinmap::DigitalPins::DMUX2_INT, GPIO_IN);
  }
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

void MCP23S17::reset() {
  gpio_put((uint)pinmap::DigitalPins::DMUX_RESET, 0);
  sleep_ms(1);
  gpio_put((uint)pinmap::DigitalPins::DMUX_RESET, 1);
}

void MCP23S17::init_gpio(uint8_t pin, bool is_output) {
  // set the direction of the pin
  uint8_t reg = 0x00;
  if (is_output) {
    reg = 0x00;
  } else {
    reg = 0xFF;
  }
  write8(pin, reg);
}