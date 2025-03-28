#include "debug.hpp"
#include "pinmap.hpp"
#include "pins/MCP23S17.hpp"
#include "types.hpp"
#include "pins/types.hpp"
#include "pins/digital_pins.hpp"

extern "C" {
#include "hardware/gpio.h"
#include <pico/stdlib.h>
}

namespace pins {

using namespace pinmap;
using namespace types;

DigitalPins digital_pins = DigitalPins();

bool DigitalPins::init() {
  if (_initialized) { // prevent multiple initialization from causing issues
    debug::warn("DigitalPins: tried to double initialize\r\n");
    return true;
  }
  debug::log("DigitalPins: initialising DigitalPins\r\n");
  // initialise dmux
  debug::log("DigitalPins: intialising dmux1\r\n");
  _dmux_1.init();
  debug::log("DigitalPins: intialising dmux2\r\n");
  _dmux_2.init();
  // add interrupt handler
  debug::log("DigitalPins: adding interrupt handler\r\n");
  gpio_set_irq_callback(pico_gpio_interrupt_handler_wrapper);
  // attach dmux interrupts
  debug::log("DigitalPins: attaching dmux2 interrupt handler\r\n");
  attach_interrupt(Digital::DMUX2_INT, DigitalPinInterruptState::LEVEL_HIGH,
                   make_handler(_dmux_2.interrupt_handler), NULL);
  debug::log("DigitalPins: attaching dmux1 interrupt handler\r\n");
  attach_interrupt(Digital::DMUX2_INT, DigitalPinInterruptState::LEVEL_HIGH,
                   make_handler(_dmux_1.interrupt_handler), NULL);
  _initialized = true;
  return true;
}

bool DigitalPins::attach_interrupt(pinmap::Digital pin,
                                   DigitalPinInterruptState interrupt_state,
                                   const DigitalPinInterrupt interrupt_handler,
                                   void *args) {
  if (!_initialized) {
    debug::fatal(
        "DigitalPins: tried attaching interrupt when unintialized\r\n");
    return false;
  }

  u8 pin_idx = (u8)pin;
  _interrupts[pin_idx] = interrupt_handler;
  _interrupt_states[pin_idx] = interrupt_state;
  _interrupt_args[pin_idx] = args;

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_dir(pin_idx, GPIO_IN);
    gpio_set_irq_enabled(pin_number(pin), (u32)_interrupt_states[pin_idx],
                         true);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.attach_interrupt(pin_number(pin), true, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.attach_interrupt(pin_number(pin), false, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.attach_interrupt(pin_number(pin), true, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.attach_interrupt(pin_number(pin), false, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  }
}

bool DigitalPins::detach_interrupt(pinmap::Digital pin) {
  if (!_initialized) {
    debug::fatal(
        "DigitalPins: tried detaching interrupt when unintialized\r\n");
    return false;
  }

  u8 pin_idx = (u8)pin;
  _interrupts[pin_idx] = nullptr;
  _interrupt_states[pin_idx] = DigitalPinInterruptState::EDGE_FALL;
  _interrupt_args[pin_idx] = nullptr;

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_irq_enabled(pin_number(pin), (u32)_interrupt_states[pin_idx],
                         false);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.detach_interrupt(pin_number(pin), true);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.detach_interrupt(pin_number(pin), false);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.detach_interrupt(pin_number(pin), true);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.detach_interrupt(pin_number(pin), false);
    break;
  }
}

bool DigitalPins::enable_interrupt(pinmap::Digital pin) {
  if (!_initialized) {
    debug::fatal("DigitalPins: tried enabling interrupt when unintialized\r\n");
    return false;
  }

  u8 pin_idx = (u8)pin;

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_irq_enabled(pin_number(pin), (u32)_interrupt_states[pin_idx],
                         true);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.enable_interrupt(pin_number(pin), true);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.enable_interrupt(pin_number(pin), false);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.enable_interrupt(pin_number(pin), true);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.enable_interrupt(pin_number(pin), false);
    break;
  }

  return true;
}

bool DigitalPins::disable_interrupt(pinmap::Digital pin) {
  if (!_initialized) {
    debug::fatal(
        "DigitalPins: tried disabling interrupt when unintialized\r\n");
    return false;
  }

  u8 pin_idx = (u8)pin;

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_irq_enabled(pin_number(pin), (u32)_interrupt_states[pin_idx],
                         false);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.disable_interrupt(pin_number(pin), true);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.disable_interrupt(pin_number(pin), false);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.disable_interrupt(pin_number(pin), true);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.disable_interrupt(pin_number(pin), false);
    break;
  }

  return true;
}

void DigitalPins::pico_gpio_interrupt_handler(u32 gpio, u32 event) {
  if (_interrupts[gpio] == nullptr) {
    return;
  }

  // call if event matches
  if (event & (u32)_interrupt_states[gpio]) {
    _interrupts[gpio](_interrupt_args[gpio]);
  }
  return;
}

} // namespace pins
