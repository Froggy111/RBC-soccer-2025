#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include <cstdint>
#include <pico/types.h>

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

// register defaults
#define IOCON_DEFAULT 0b00001000

// spi masks
#define SPI_CMD_DEFAULT 0b01000000

void MCP23S17::init(types::u8 device_id, spi_inst_t *spi_obj_touse) {
  printf("-> Initializing MCP23S17\n");
  if (device_id != 1 && device_id != 2) {
    printf("Error: Invalid device ID\n");
    return;
  }
  id = device_id;

  memset(pin_state, 0, sizeof(pin_state));

  // init gpio pins
  printf("Initializing pins\n");
  init_pins();

  // reset using the reset pin
  reset();

  // init spi
  printf("Initializing SPI\n");
  spi_obj = spi_obj_touse;
  init_spi();
}

void MCP23S17::init_pins() {
  // Initialize RESET
  gpio_init((uint)pinmap::DigitalPins::DMUX_RESET);
  gpio_set_dir((uint)pinmap::DigitalPins::DMUX_RESET, GPIO_OUT);

  // initialize INTA
  if (id == 2) {
    gpio_init((uint)pinmap::DigitalPins::DMUX2_INT);
    gpio_set_dir((uint)pinmap::DigitalPins::DMUX2_INT, GPIO_IN);
  }
}

void MCP23S17::init_spi() {
  configure_spi(spi_obj);
  
  // Enable Addressing via Address Pins
  // Since all resetted MCPs will have HAEN = 0, sending with a random address
  // will cause it to affect all HAEN = 0 MCPs
  write8(0, IOCON, IOCON_DEFAULT);
}

void MCP23S17::write8(uint8_t device_address, uint8_t reg_address, uint8_t data,
                      uint8_t mask) {
  configure_spi(spi_obj);
  uint8_t current = read8(device_address, reg_address);

  uint8_t tx_data[3] = {(uint8_t)(SPI_CMD_DEFAULT | (device_address << 1)),
                        reg_address,
                        (uint8_t)((current & ~mask) | (data & mask))};

  gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 0);
  spi_write_blocking(spi0, tx_data, 3);
  gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 1);
}

uint8_t MCP23S17::read8(uint8_t device_address, uint8_t reg_address) {
  configure_spi(spi_obj);
  uint8_t tx_data[3] = {
      (uint8_t)(SPI_CMD_DEFAULT | (device_address << 1) | 0b1), reg_address, 0};

  uint8_t rx_data;

  gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 0);
  spi_write_read_blocking(spi0, tx_data, &rx_data, 3);
  gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, 1);

  return rx_data;
}

void MCP23S17::configure_spi(spi_inst_t *spi_obj) {
  // Initialize SPI pins (except CS)
  gpio_set_function((uint)pinmap::DigitalPins::SPI0_SCLK, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::DigitalPins::SPI0_MOSI, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::DigitalPins::SPI0_MISO, GPIO_FUNC_SPI);
    
  // Initialize CS pin as GPIO
  gpio_init((uint)pinmap::DigitalPins::DMUX_SCS);
  gpio_set_dir((uint)pinmap::DigitalPins::DMUX_SCS, GPIO_OUT);
  gpio_put((uint)pinmap::DigitalPins::DMUX_SCS, DEFAULT_CS);

  // Set SPI format
  spi_set_format(spi_obj, 16, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
}

void MCP23S17::reset() {
  gpio_put((uint)pinmap::DigitalPins::DMUX_RESET, 0);
  sleep_ms(1);
  gpio_put((uint)pinmap::DigitalPins::DMUX_RESET, 1);
}

void MCP23S17::init_gpio(uint8_t pin, bool on_A, bool is_output) {
  printf("Pin: %d, on_A: %d, is_output: %d\n", pin, on_A, is_output);
  if (pin < 0 || pin > 7) {
    printf("Error: Invalid pin number\n");
    return;
  }

  // configure direction
  write8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? IODIRA : IODIRB, !is_output << pin, 0b1 << pin);
  pin_state[pin + (on_A ? 0 : 8)] = is_output;
}

void MCP23S17::write_gpio(uint8_t pin, bool on_A, bool value) {
  if (pin < 0 || pin > 7) {
    printf("Error: Invalid pin number\n");
    return;
  }

  if (pin_state[pin + (on_A ? 0 : 8)] != 1) {
    printf("Error: Pin is not configured as output\n");
    return;
  }

  // write to the pin
  write8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? GPIOA : GPIOB,  value << pin, 0b1 << pin);
}

bool MCP23S17::read_gpio(uint8_t pin, bool on_A) {
  if (pin < 0 || pin > 7) {
    printf("Error: Invalid pin number\n");
    return false;
  }

  if (pin_state[pin + (on_A ? 0 : 8)] != 0) {
    printf("Error: Pin is not configured as input\n");
    return false;
  }

  // read the pin
  return read8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? GPIOA : GPIOB) & (1 << pin);
}