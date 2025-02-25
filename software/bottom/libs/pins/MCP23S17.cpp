#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"

extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include <stdio.h>
#include <string.h>
}

// addresses of the MCP23S17
#define ADDRESS_1 0b000
#define ADDRESS_2 0b001

// default values for pins
#define DEFAULT_CS 1 // CS high by default

// register addresses (we default to BANK = 0)
#define IODIRA 0x00
#define IODIRB 0x01
#define IPOLA 0x02 // NOT USING
#define IPOLB 0x03 // NOT USING
#define GPINTENA 0x04
#define GPINTENB 0x05
#define DEFVALA 0x06
#define DEFVALB 0x07
#define INTCONA 0x08
#define INTCONB 0x09
#define IOCON 0x0A
#define GPPUA 0x0C
#define GPPUB 0x0D
#define INTFA 0x0E
#define INTFB 0x0F
#define INTCAPA 0x10
#define INTCAPB 0x11
#define GPIOA 0x12
#define GPIOB 0x13
#define OLATA 0x14
#define OLATB 0x15

void MCP23S17::init(types::u8 device_id, types::u32 baudrate) {
  printf("---> Initializing MCP23S17\n\n");
  if (device_id != 1 && device_id != 2) {
    printf("Error: Invalid device ID\n");
    return;
  }
  id = device_id;

  memset(pin_state, 0, sizeof(pin_state));

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
  // TODO
    gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 0);

    gpio_put((uint) pinmap::DigitalPins::DMUX_SCS, 1);
}

uint8_t MCP23S17::read8(uint8_t reg_address) {
  // TODO
  gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 0);

  gpio_put((uint) pinmap::DigitalPins::DMUX_SCS, 1);
}

void MCP23S17::reset() {
  gpio_put((uint)pinmap::DigitalPins::DMUX_RESET, 0);
  sleep_ms(1);
  gpio_put((uint)pinmap::DigitalPins::DMUX_RESET, 1);
}

void MCP23S17::init_gpio(uint8_t pin, bool on_A, bool is_output) {
  if (pin < 0 || pin > 7) {
    printf("Error: Invalid pin number\n");
    return;
  }

  // configure direction
  if (on_A) {
    write8(IODIRA, !is_output << pin);
    pin_state[pin] = is_output;
  } else {
    write8(IODIRB, !is_output << pin);
    pin_state[pin + 8] = is_output;
  }
}

void MCP23S17::write_gpio(uint8_t pin, bool on_A, bool value) {
  if (pin < 0 || pin > 7) {
    printf("Error: Invalid pin number\n");
    return;
  }

  if (pin_state[pin] != 1) {
    printf("Error: Pin is not configured as output\n");
    return;
  }

  // write to the pin
  if (on_A) {
    write8(GPIOA, value << pin);
  } else {
    write8(GPIOB, value << pin);
  }
}

bool MCP23S17::read_gpio(uint8_t pin, bool on_A) {
  if (pin < 0 || pin > 7) {
    printf("Error: Invalid pin number\n");
    return false;
  }

  if (pin_state[pin] != 0) {
    printf("Error: Pin is not configured as input\n");
    return false;
  }

  // read the pin
  if (on_A) {
    return read8(GPIOA) & (1 << pin);
  } else {
    return read8(GPIOB) & (1 << pin);
  }
}