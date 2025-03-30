#include "pins/MCP23S17.hpp"
#include "debug.hpp"
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

// default values for pins
const u8 DEFAULT_CS = 1; // CS high by default

// register addresses (we default to BANK = 0)
const u8 IODIRA = 0x00;
const u8 IODIRB = 0x01;
const u8 IPOLA = 0x02; // NOT USING
const u8 IPOLB = 0x03; // NOT USING
const u8 GPINTENA = 0x04;
const u8 GPINTENB = 0x05;
const u8 DEFVALA = 0x06;
const u8 DEFVALB = 0x07;
const u8 INTCONA = 0x08;
const u8 INTCONB = 0x09;
const u8 IOCON = 0x0A;
const u8 GPPUA = 0x0C;
const u8 GPPUB = 0x0D;
const u8 INTFA = 0x0E;
const u8 INTFB = 0x0F;
const u8 INTCAPA = 0x10;
const u8 INTCAPB = 0x11;
const u8 GPIOA = 0x12;
const u8 GPIOB = 0x13;
const u8 OLATA = 0x14;
const u8 OLATB = 0x15;

// register defaults
const u8 IOCON_DEFAULT = 0b00001000;

// spi masks
const u8 SPI_CMD_DEFAULT = 0b01000000;

const u16 INTERRUPT_TASK_STACK_SIZE = 256;
const u8 INTERRUPT_TASK_PRIORITY = 12;

MCP23S17::MCP23S17(u8 SCLK, u8 MISO, u8 MOSI, u8 SCS, u8 RESET, u8 address,
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
}

void MCP23S17::init() {
  debug::debug("-> Initializing MCP23S17 with address %u\r\n", _address);

  // init gpio pins
  debug::debug("Initializing pins\r\n");
  init_pins();

  // reset using the reset pin
  reset();

  // init spi
  debug::debug("Initializing SPI\r\n");
  init_spi();

  // // set interrupt condition to INTPOL
  // write8(IOCON, 0b00000010, 0b00000010);
  // // set active drive for interrupt
  // write8(IOCON, 0b00000000, 0b00000100);
  // // set interrupts to all go to INTA
  // write8(IOCON, 0b01000000, 0b01000000);

  // create interrupt handling task
  // debug::debug("Creating interrupt handling task\r\n");
  // xTaskCreate(interrupt_handler_task, "MCP23S17 interrupt handling task",
  //             INTERRUPT_TASK_STACK_SIZE, this, INTERRUPT_TASK_PRIORITY,
  //             &interrupt_handler_task_handle);
  return;
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

void MCP23S17::write8(u8 reg_address, u8 data, u8 mask) {
  configure_spi();
  u8 current = read8(reg_address);
  u8 tx_data[3] = {(u8)(SPI_CMD_DEFAULT | (_address << 1)), reg_address,
                   (u8)((current & ~mask) | (data & mask))};

  gpio_put((uint)pinmap::Pico::DMUX_SCS, 0);
  spi_write_blocking(_spi_obj, tx_data, 3);
  gpio_put((uint)pinmap::Pico::DMUX_SCS, 1);

  // read register and check if it was written
  u8 res = read8(reg_address);
  // for (u8 i = 0; i < 3; i++) {
  //   for (u8 j = 7; j >= 0; j--) {
  //     comms::USB_CDC.printf("%u", (tx_data[i] >> j) & 1);
  //   }
  //   debug::debug(" ");
  // }
  if (res != tx_data[2]) {
    // print tx_data in binary
    for (u8 i = 0; i < 3; i++) {
      for (u8 j = 7; j >= 0; j--) {
        debug::debug("%u", (tx_data[i] >> j) & 1);
      }
      debug::debug(" ");
    }
    debug::error("MCP23S17 write8 to register failed. Expected %u, got %u\r\n",
                 tx_data[2], res);
  }
}

u8 MCP23S17::read8(u8 reg_address) {
  configure_spi();
  u8 tx_data[2] = {(u8)(SPI_CMD_DEFAULT | (_address << 1) | 0b1), reg_address};

  u8 rx_data;

  gpio_put((uint)pinmap::Pico::DMUX_SCS, 0);
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

void MCP23S17::set_pin_mode(u8 pin, bool on_A, DigitalPinMode pinmode) {
  if (pin < 0 || pin > 7) {
    debug::error("Error: Invalid pin number: %u\r\n", pin);
    return;
  }

  switch (pinmode) {
  case DigitalPinMode::INPUT:
    write8(on_A ? IODIRA : IODIRB, 1 << pin, 0b1 << pin);
    write8(on_A ? GPPUA : GPPUB, 0 << pin, 0b1 << pin); // dont pullup
    _pin_state[pin + (on_A ? 0 : 8)] = 1;               // input
    break;
  case DigitalPinMode::INPUT_PULLUP:
    write8(on_A ? IODIRA : IODIRB, 1 << pin, 0b1 << pin);
    write8(on_A ? GPPUA : GPPUB, 1 << pin, 0b1 << pin); // pullup
    _pin_state[pin + (on_A ? 0 : 8)] = 1;               // input
    break;
  case DigitalPinMode::OUTPUT:
    write8(on_A ? IODIRA : IODIRB, 0 << pin, 0b1 << pin);
    _pin_state[pin + (on_A ? 0 : 8)] = 0; // output
    break;
  }
}

void MCP23S17::write(u8 pin, bool on_A, bool value) {
  if (pin < 0 || pin > 7) {
    debug::error("Error: Invalid pin number: %u\r\n", pin);
    return;
  }

  if (_pin_state[pin + (on_A ? 0 : 8)] != 0) {
    debug::error("Error: Pin %u on_A %u is not configured as output\r\n", pin,
                 on_A);
    return;
  }

  // write to the pin
  write8(on_A ? GPIOA : GPIOB, value << pin, 0b1 << pin);

  debug::debug(
      "MCP23S17 address %u wrote to pin %u on bus %u with value %u\r\n",
      _address, pin, on_A, value);

  // check that OUTPUT_LATCH has the written bit
  u8 res = read8(on_A ? OLATA : OLATB);

  if ((res & (1 << pin)) != (value << pin)) {
    debug::error(
        "ERROR: Write to MCP23S17 GPIO failed. Expected %u, got %u\r\n",
        value << pin, res);
  }
}

bool MCP23S17::read(u8 pin, bool on_A) {
  if (pin < 0 || pin > 7) {
    debug::error("Error: Invalid pin number: %u\r\n", pin);
    return false;
  }

  if (_pin_state[pin + (on_A ? 0 : 8)] != 1) {
    debug::error("Error: Pin %u ID %u on_A %u is not configured as input\r\n",
                 pin, on_A);
    return false;
  }

  // read the pin
  uint8_t res = read8(on_A ? GPIOA : GPIOB) & (1 << pin);
  debug::debug("MCP23S17 address %u read pin %u on bus %u with value %u\r\n",
               _address, pin, on_A, res);
  return res;
}

// bool MCP23S17::attach_interrupt(u8 pin, bool on_A,
//                                 DigitalPinInterrupt interrupt_fn,
//                                 DigitalPinInterruptState interrupt_state,
//                                 void *interrupt_fn_params) {
//   if (pin < 0 || pin > 7) {
//     comms::USB_CDC.printf("Error: Invalid pin number: %u\r\n", pin);
//     return false;
//   }
//   if (_pin_state[pin + (on_A ? 0 : 8)] != 1) {
//     comms::USB_CDC.printf(
//         "Error: Pin %u ID %u on_A %u is not configured as input\r\n", pin,
//         on_A);
//     return false;
//   }
//
//   // configure interrupt condition
//   // disable interrupt on that pin first
//   write8(on_A ? GPINTENA : GPINTENB, 0 << pin, 0b1 << pin);
//   switch (interrupt_state) {
//   case DigitalPinInterruptState::EDGE_FALL:
//     // default value high, interrupt on change
//     write8(on_A ? DEFVALA : DEFVALB, 1 << pin, 0b1 << pin);
//     write8(on_A ? INTCONA : INTCONB, 0 << pin, 0b1 << pin);
//     _interrupt_states[pin + (on_A ? 0 : 8)] =
//         DigitalPinInterruptState::EDGE_FALL;
//     break;
//   case DigitalPinInterruptState::EDGE_RISE:
//     // default value high, interrupt on change
//     write8(on_A ? DEFVALA : DEFVALB, 1 << pin, 0b1 << pin);
//     write8(on_A ? INTCONA : INTCONB, 0 << pin, 0b1 << pin);
//     _interrupt_states[pin + (on_A ? 0 : 8)] =
//         DigitalPinInterruptState::EDGE_RISE;
//     break;
//   case DigitalPinInterruptState::LEVEL_HIGH:
//     // default value high, interrupt on change
//     write8(on_A ? DEFVALA : DEFVALB, 0 << pin, 0b1 << pin);
//     write8(on_A ? INTCONA : INTCONB, 1 << pin, 0b1 << pin);
//     _interrupt_states[pin + (on_A ? 0 : 8)] =
//         DigitalPinInterruptState::LEVEL_HIGH;
//     break;
//   case DigitalPinInterruptState::LEVEL_LOW:
//     // default value high, interrupt on change
//     write8(on_A ? DEFVALA : DEFVALB, 1 << pin, 0b1 << pin);
//     write8(on_A ? INTCONA : INTCONB, 1 << pin, 0b1 << pin);
//     _interrupt_states[pin + (on_A ? 0 : 8)] =
//         DigitalPinInterruptState::LEVEL_LOW;
//     break;
//   }
//   _interrupt_handlers[pin + (on_A ? 0 : 8)] = interrupt_fn;
//   // enable interrupt on that pin
//   write8(on_A ? GPINTENA : GPINTENB, 1 << pin, 0b1 << pin);
//   return true;
// }
//
// void MCP23S17::detach_interrupt(u8 pin, bool on_A) {
//   disable_interrupt(pin, on_A);
//   _interrupt_handlers[pin + (on_A ? 0 : 8)] = nullptr;
// }
//
// void MCP23S17::disable_interrupt(u8 pin, bool on_A) {
//   write8(on_A ? GPINTENA : GPINTENB, 0 << pin, 0b1 << pin);
// }
//
// void MCP23S17::enable_interrupt(u8 pin, bool on_A) {
//   write8(on_A ? GPINTENA : GPINTENB, 1 << pin, 0b1 << pin);
// }
//
// void MCP23S17::interrupt_handler(void *params) {
//   if (_int_from_isr) {
//     BaseType_t higher_priority_task_woken;
//     xTaskNotifyFromISR(interrupt_handler_task_handle, 0, eNoAction,
//                        &higher_priority_task_woken);
//     portYIELD_FROM_ISR(higher_priority_task_woken);
//   } else {
//     xTaskNotify(interrupt_handler_task_handle, 0, eNoAction);
//   }
// }
//
// void MCP23S17::interrupt_handler_task(void *params) {
//   MCP23S17 *_this = (MCP23S17 *)params;
//   for (;;) {
//     ulTaskNotifyTake(
//         pdTRUE, portMAX_DELAY); // block until notified by interrupt handler
//     // read INTFA and INTFB
//     u8 int_flags_a = _this->read8(INTFA);
//     u8 int_flags_b = _this->read8(INTFB);
//     u8 int_cap_a = _this->read8(INTCAPA);
//     u8 int_cap_b = _this->read8(INTCAPB);
//
//     // see which pins caused interrupt
//     // bus A
//     if (int_flags_a) {
//       for (u8 pin = 0; pin < 8; pin++) {
//         if (!_this->_interrupt_handlers[pin]) {
//           // complain (very loudly)
//           debug::fatal("interrupt on pin %u without handler attached in \
// MCP23S17 with address %u\r\n",
//                        pin, _this->_address);
//           continue; // handle gracefully
//         }
//         u8 pin_mask = (1 << pin);
//         if (int_flags_a & pin_mask) { // pin flagged
//           // check if int_cap has the same trigger condition as attached
//           bool pin_state = int_cap_a & pin_mask;
//           switch (_this->_interrupt_states[pin]) {
//           case DigitalPinInterruptState::LEVEL_HIGH:
//             if (pin_state) {
//               _this->_interrupt_handlers[pin](
//                   _this->_interrupt_handler_args[pin]);
//             }
//             break;
//           case DigitalPinInterruptState::LEVEL_LOW:
//             if (!pin_state) {
//               _this->_interrupt_handlers[pin](
//                   _this->_interrupt_handler_args[pin]);
//             }
//             break;
//           case DigitalPinInterruptState::EDGE_RISE:
//             if (pin_state) {
//               _this->_interrupt_handlers[pin](
//                   _this->_interrupt_handler_args[pin]);
//             }
//             break;
//           case DigitalPinInterruptState::EDGE_FALL:
//             if (!pin_state) {
//               _this->_interrupt_handlers[pin](
//                   _this->_interrupt_handler_args[pin]);
//             }
//             break;
//           }
//         }
//       }
//     }
//
//     // bus B
//     if (int_flags_b) {
//       for (u8 pin = 0; pin < 8; pin++) {
//         u8 pin_mask = (1 << pin);
//         if (int_flags_b & pin_mask) { // pin flagged
//           // check if int_cap has the same trigger condition as attached
//           bool pin_state = int_cap_a & pin_mask;
//           switch (_this->_interrupt_states[pin + 8]) {
//           case DigitalPinInterruptState::LEVEL_HIGH:
//             if (pin_state) {
//               _this->_interrupt_handlers[pin + 8](
//                   _this->_interrupt_handler_args[pin + 8]);
//             }
//             break;
//           case DigitalPinInterruptState::LEVEL_LOW:
//             if (!pin_state) {
//               _this->_interrupt_handlers[pin + 8](
//                   _this->_interrupt_handler_args[pin + 8]);
//             }
//             break;
//           case DigitalPinInterruptState::EDGE_RISE:
//             if (pin_state) {
//               _this->_interrupt_handlers[pin + 8](
//                   _this->_interrupt_handler_args[pin + 8]);
//             }
//             break;
//           case DigitalPinInterruptState::EDGE_FALL:
//             if (!pin_state) {
//               _this->_interrupt_handlers[pin + 8](
//                   _this->_interrupt_handler_args[pin + 8]);
//             }
//             break;
//           }
//         }
//       }
//     }
//   }
// }

} // namespace pins
