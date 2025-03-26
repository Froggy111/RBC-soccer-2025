#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include <pico/types.h>
#include "comms.hpp"

extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
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

bool MCP23S17::initialized[2] = {false, false};

void MCP23S17::init(types::u8 device_id, spi_inst_t *spi_obj_touse) {
  if (device_id != 1 && device_id != 2) {
    comms::USB_CDC.printf("Error: Invalid device ID\r\n");
    return;
  }

  id = device_id;
  spi_obj = spi_obj_touse;
  memset(pin_state, 0, sizeof(pin_state));
  if (initialized[device_id - 1]) {
    return;
  }
  initialized[device_id - 1] = true;

  comms::USB_CDC.printf("-> Initializing MCP23S17\r\n");

  // init gpio pins
  comms::USB_CDC.printf("Initializing pins\r\n");
  init_pins();

  // reset using the reset pin
  reset();

  // init spi
  comms::USB_CDC.printf("Initializing SPI\r\n");
  init_spi();
}

void MCP23S17::init_pins() {
  // Initialize RESET
  gpio_init((uint)pinmap::Pico::DMUX_RESET);
  gpio_set_dir((uint)pinmap::Pico::DMUX_RESET, GPIO_OUT);

  // TODO: initialize INTA
}

void MCP23S17::init_spi() {
  configure_spi();

  // Enable Addressing via Address Pins
  // Since all resetted MCPs will have HAEN = 0, sending with a random address
  // will cause it to affect all HAEN = 0 MCPs
  write8(0, IOCON, IOCON_DEFAULT);
}

void MCP23S17::write8(uint8_t device_address, uint8_t reg_address, uint8_t data,
                      uint8_t mask) {
  configure_spi();
  uint8_t current = read8(device_address, reg_address);
  uint8_t tx_data[3] = {(uint8_t)(SPI_CMD_DEFAULT | (device_address << 1)),
                        reg_address,
                        (uint8_t)((current & ~mask) | (data & mask))};

  gpio_put((uint)pinmap::Pico::DMUX_SCS, 0);
  spi_write_blocking(spi_obj, tx_data, 3);
  gpio_put((uint)pinmap::Pico::DMUX_SCS, 1);

  // read register and check if it was written
  uint8_t res = read8(device_address, reg_address);
  if (res != tx_data[2]) {
    // print tx_data in binary
    for (int i = 0; i < 3; i++) {
      for (int j = 7; j >= 0; j--) {
        comms::USB_CDC.printf("%d", (tx_data[i] >> j) & 1);
      }
      comms::USB_CDC.printf(" ");
    }
    comms::USB_CDC.printf(
        "ERROR: MCP23S17 write8 to register failed. Expected %d, got %d\r\n",
        tx_data[2], res);
  }
}

uint8_t MCP23S17::read8(uint8_t device_address, uint8_t reg_address) {
  configure_spi();
  uint8_t tx_data[2] = {
      (uint8_t)(SPI_CMD_DEFAULT | (device_address << 1) | 0b1), reg_address};

  uint8_t rx_data;

  gpio_put((uint)pinmap::Pico::DMUX_SCS, 0);
  spi_write_blocking(spi_obj, tx_data, 2);
  spi_read_blocking(spi_obj, 0xFF, &rx_data, 1);
  gpio_put((uint)pinmap::Pico::DMUX_SCS, 1);

  return rx_data;
}

void MCP23S17::configure_spi() {
  // Initialize SPI pins (except CS)
  gpio_set_function((uint)pinmap::Pico::SPI0_SCLK, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MOSI, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MISO, GPIO_FUNC_SPI);

  // Initialize CS pin as GPIO
  gpio_init((uint)pinmap::Pico::DMUX_SCS);
  gpio_set_dir((uint)pinmap::Pico::DMUX_SCS, GPIO_OUT);
  gpio_put((uint)pinmap::Pico::DMUX_SCS, DEFAULT_CS);

  // Set SPI format
  spi_set_format(spi_obj, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void MCP23S17::reset() {
  gpio_put((uint)pinmap::Pico::DMUX_RESET, 0);
  sleep_ms(1);
  gpio_put((uint)pinmap::Pico::DMUX_RESET, 1);
}

void MCP23S17::init_gpio(uint8_t pin, bool on_A, bool is_output) {
  if (pin < 0 || pin > 7) {
    comms::USB_CDC.printf("Error: Invalid pin number: %d\r\n", pin);
    return;
  }

  comms::USB_CDC.printf("InitializingA pin %d on %s as %s\r\n", pin,
                        on_A ? "GPIOA" : "GPIOB",
                        is_output ? "OUTPUT" : "INPUT");

  // configure direction
  write8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? IODIRA : IODIRB,
         !is_output << pin, 0b1 << pin);

  pin_state[pin + (on_A ? 0 : 8)] = is_output;
}

void MCP23S17::write_gpio(uint8_t pin, bool on_A, bool value) {
  if (pin < 0 || pin > 7) {
    comms::USB_CDC.printf("Error: Invalid pin number: %d\r\n", pin);
    return;
  }

  if (pin_state[pin + (on_A ? 0 : 8)] != 1) {
    comms::USB_CDC.printf(
        "Error: Pin %d ID %d on_A %d is not configured as output\r\n", pin, id,
        on_A);
    return;
  }

  // write to the pin
  write8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? GPIOA : GPIOB, value << pin,
         0b1 << pin);

  // comms::USB_CDC.printf("Driver ID: %d, A: %d, Wrote to pin %d with value %d\r\n", id, on_A, pin, value);

  // check that OUTPUT_LATCH has the written bit
  uint8_t res = read8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? OLATA : OLATB);

  if ((res & (1 << pin)) != (value << pin)) {
    comms::USB_CDC.printf(
        "ERROR: Write to MCP23S17 GPIO failed. Expected %d, got %d\r\n",
        value << pin, res);
  }
}

bool MCP23S17::read_gpio(uint8_t pin, bool on_A) {
  if (pin < 0 || pin > 7) {
    comms::USB_CDC.printf("Error: Invalid pin number: %d\r\n", pin);
    return false;
  }

  if (pin_state[pin + (on_A ? 0 : 8)] != 0) {
    comms::USB_CDC.printf(
        "Error: Pin %d ID %d on_A %d is not configured as input\r\n", pin, id,
        on_A);
    return false;
  }

  // read the pin
  return read8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? GPIOA : GPIOB) &
         (1 << pin);
}

void MCP23S17::pullup_gpio(uint8_t pin, bool on_A) {
  if (pin < 0 || pin > 7) {
    comms::USB_CDC.printf("Error: Invalid pin number: %d\r\n", pin);
  }

  write8(id == 1 ? ADDRESS_1 : ADDRESS_2, on_A ? GPPUA : GPPUB, 1 << pin,
         0b1 << pin);
}
