#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include "comms.hpp"

extern "C" {
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <hardware/timer.h>
#include <pico/types.h>
#include <pico/time.h>
}

using namespace types;

namespace pins {

// addresses of the MCP23S17
const types::u8 ADDRESS_1 = 0b000;
const types::u8 ADDRESS_2 = 0b001;

// default values for pins
const types::u8 DEFAULT_CS = 1; // CS high by default

// register addresses (we default to BANK = 0)
const types::u8 IODIRA = 0x00;
const types::u8 IODIRB = 0x01;
const types::u8 IPOLA = 0x02; // NOT USING
const types::u8 IPOLB = 0x03; // NOT USING
const types::u8 GPINTENA = 0x04;
const types::u8 GPINTENB = 0x05;
const types::u8 DEFVALA = 0x06;
const types::u8 DEFVALB = 0x07;
const types::u8 INTCONA = 0x08;
const types::u8 INTCONB = 0x09;
const types::u8 IOCON = 0x0A;
const types::u8 GPPUA = 0x0C;
const types::u8 GPPUB = 0x0D;
const types::u8 INTFA = 0x0E;
const types::u8 INTFB = 0x0F;
const types::u8 INTCAPA = 0x10;
const types::u8 INTCAPB = 0x11;
const types::u8 GPIOA = 0x12;
const types::u8 GPIOB = 0x13;
const types::u8 OLATA = 0x14;
const types::u8 OLATB = 0x15;

// register defaults
const types::u8 IOCON_DEFAULT = 0b00001000;

// spi masks
const types::u8 SPI_CMD_DEFAULT = 0b01000000;

void MCP23S17::init(u8 SCLK, u8 MISO, u8 MOSI, u8 SCS, u8 RESET, u8 address,
                    bool int_from_isr, spi_inst_t *spi_obj) {
  _spi_obj = spi_obj;
  memset(_pin_state, 0, sizeof(_pin_state));
  // prevent multiple init
  if (_initialised) {
    return;
  }

  _SCLK = SCLK;
  _MISO = MISO;
  _MOSI = MOSI;
  _SCS = SCS;
  _RESET = RESET;
  _address = address;
  _int_from_isr = int_from_isr;

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
  gpio_init(_RESET);
  gpio_set_dir(_RESET, GPIO_OUT);
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
  busy_wait_us(1);
  spi_write_blocking(_spi_obj, tx_data, 3);
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
  busy_wait_us(1);
  spi_write_blocking(_spi_obj, tx_data, 2);
  spi_read_blocking(_spi_obj, 0xFF, &rx_data, 1);
  gpio_put((uint)pinmap::Pico::DMUX_SCS, 1);

  return rx_data;
}

void MCP23S17::configure_spi() {
  // Initialize SPI pins (except CS)
  gpio_set_function(_SCLK, GPIO_FUNC_SPI);
  gpio_set_function(_MOSI, GPIO_FUNC_SPI);
  gpio_set_function(_MISO, GPIO_FUNC_SPI);

  // Initialize CS pin as GPIO
  gpio_init(_SCS);
  gpio_set_dir(_SCS, GPIO_OUT);
  gpio_put(_SCS, DEFAULT_CS);

  // Set SPI format
  spi_set_format(_spi_obj, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void MCP23S17::reset() {
  gpio_put(_RESET, 0);
  sleep_ms(1);
  gpio_put(_RESET, 1);
}

void MCP23S17::set_pin_mode(uint8_t pin, bool on_A, DigitalPinMode pinmode) {
  if (pin < 0 || pin > 7) {
    comms::USB_CDC.printf("Error: Invalid pin number: %d\r\n", pin);
    return;
  }

  switch (pinmode) {
  case DigitalPinMode::INPUT:
    write8(_address, on_A ? IODIRA : IODIRB, 1 << pin, 0b1 << pin);
    write8(_address, on_A ? GPPUA : GPPUB, 0 << pin, 0b1 << pin); // dont pullup
    _pin_state[pin + (on_A ? 0 : 8)] = 1;                         // input
    break;
  case DigitalPinMode::INPUT_PULLUP:
    write8(_address, on_A ? IODIRA : IODIRB, 1 << pin, 0b1 << pin);
    write8(_address, on_A ? GPPUA : GPPUB, 1 << pin, 0b1 << pin); // pullup
    _pin_state[pin + (on_A ? 0 : 8)] = 1;                         // input
    break;
  case DigitalPinMode::OUTPUT:
    write8(_address, on_A ? IODIRA : IODIRB, 0 << pin, 0b1 << pin);
    _pin_state[pin + (on_A ? 0 : 8)] = 0; // output
    break;
  }
}

void MCP23S17::write(uint8_t pin, bool on_A, bool value) {
  if (pin < 0 || pin > 7) {
    comms::USB_CDC.printf("Error: Invalid pin number: %d\r\n", pin);
    return;
  }

  if (_pin_state[pin + (on_A ? 0 : 8)] != 0) {
    comms::USB_CDC.printf(
        "Error: Pin %d on_A %d is not configured as output\r\n", pin, on_A);
    return;
  }

  // write to the pin
  write8(_address, on_A ? GPIOA : GPIOB, value << pin, 0b1 << pin);

  // comms::USB_CDC.printf("Driver ID: %d, A: %d, Wrote to pin %d with value %d\r\n", id, on_A, pin, value);

  // check that OUTPUT_LATCH has the written bit
  uint8_t res = read8(_address, on_A ? OLATA : OLATB);

  if ((res & (1 << pin)) != (value << pin)) {
    comms::USB_CDC.printf(
        "ERROR: Write to MCP23S17 GPIO failed. Expected %d, got %d\r\n",
        value << pin, res);
  }
}

bool MCP23S17::read(uint8_t pin, bool on_A) {
  if (pin < 0 || pin > 7) {
    comms::USB_CDC.printf("Error: Invalid pin number: %d\r\n", pin);
    return false;
  }

  if (_pin_state[pin + (on_A ? 0 : 8)] != 1) {
    comms::USB_CDC.printf(
        "Error: Pin %d ID %d on_A %d is not configured as input\r\n", pin,
        on_A);
    return false;
  }

  // read the pin
  return read8(_address, on_A ? GPIOA : GPIOB) & (1 << pin);
}

void interrupt_handler(bool in_ISR) {}

} // namespace pins
