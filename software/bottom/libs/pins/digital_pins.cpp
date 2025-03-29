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
  debug::debug("DigitalPins: initialising DigitalPins\r\n");
  // initialise dmux
  debug::debug("DigitalPins: intialising dmux1\r\n");
  _dmux_1.init();
  debug::debug("DigitalPins: intialising dmux2\r\n");
  _dmux_2.init();
  // add interrupt handler
  debug::debug("DigitalPins: adding interrupt handler\r\n");
  gpio_set_irq_callback(pico_gpio_interrupt_handler_wrapper);
  // attach dmux interrupts
  debug::debug("DigitalPins: attaching dmux2 interrupt handler\r\n");
  attach_interrupt(Digital::DMUX2_INT, DigitalPinInterruptState::LEVEL_HIGH,
                   make_handler(_dmux_2.interrupt_handler), NULL);
  debug::debug("DigitalPins: attaching dmux1 interrupt handler\r\n");
  attach_interrupt(Digital::DMUX2_INT, DigitalPinInterruptState::LEVEL_HIGH,
                   make_handler(_dmux_1.interrupt_handler), NULL);
  _initialized = true;
  return true;
}

bool DigitalPins::set_mode(pinmap::Digital pin, DigitalPinMode pin_mode) {
  if (!_initialized) {
    debug::fatal("DigitalPins: tried setting pin mode when unintialized\r\n");
    return false;
  }

  u8 pin_val = pin_number(pin);

  debug::debug("DigitalPins: setting mode of pin %u\r\n", (u8)pin);
  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_init(pin_val);
    switch (pin_mode) {
    case DigitalPinMode::INPUT:
      gpio_set_dir(pin_val, GPIO_IN);
      break;
    case DigitalPinMode::OUTPUT:
      gpio_set_dir(pin_val, GPIO_OUT);
      break;
    case DigitalPinMode::INPUT_PULLUP:
      gpio_set_dir(pin_val, GPIO_IN);
      gpio_pull_up(pin_val);
      break;
    }
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.set_pin_mode(pin_val, true, pin_mode);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.set_pin_mode(pin_val, false, pin_mode);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.set_pin_mode(pin_val, true, pin_mode);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.set_pin_mode(pin_val, false, pin_mode);
    break;
  }

  _pin_modes[(u8)pin] = pin_mode;

  return true;
}

bool DigitalPins::read(pinmap::Digital pin) {
  if (!_initialized) {
    debug::fatal("DigitalPins: tried reading pin when unintialized\r\n");
    return false;
  }
  if (_pin_modes[(u8)pin] != DigitalPinMode::INPUT &&
      _pin_modes[(u8)pin] != DigitalPinMode::INPUT_PULLUP) {
    debug::fatal("DigitalPins: tried reading pin when not set to input\r\n");
    return false;
  }

  u8 pin_val = pin_number(pin);
  bool read_val = false;

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    read_val = gpio_get(pin_val);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    read_val = _dmux_1.read(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    read_val = _dmux_1.read(pin_val, false);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    read_val = _dmux_2.read(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    read_val = _dmux_2.read(pin_val, false);
    break;
  }
  debug::debug("DigitalPins: read pin %u with value %u\r\n", (u8)pin, read_val);
  return read_val;
}

bool DigitalPins::write(pinmap::Digital pin, bool val) {
  if (!_initialized) {
    debug::fatal("DigitalPins: tried writing pin when unintialized\r\n");
    return false;
  }
  if (_pin_modes[(u8)pin] != DigitalPinMode::OUTPUT) {
    debug::fatal("DigitalPins: tried writing pin when not set to output\r\n");
    return false;
  }

  debug::debug("DigitalPins: writing pin %u with value %u\r\n", (u8)pin, val);

  u8 pin_val = pin_number(pin);

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_put(pin_val, val);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.write(pin_val, true, val);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.write(pin_val, false, val);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.write(pin_val, true, val);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.write(pin_val, false, val);
    break;
  }

  return true;
}

bool DigitalPins::attach_interrupt(pinmap::Digital pin,
                                   DigitalPinInterruptState interrupt_state,
                                   const DigitalPinInterrupt interrupt_handler,
                                   void *args) {

  u8 pin_idx = (u8)pin;
  u8 pin_val = pin_number(pin);
  _interrupts[pin_idx] = interrupt_handler;
  _interrupt_states[pin_idx] = interrupt_state;
  _interrupt_args[pin_idx] = args;

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_dir(pin_idx, GPIO_IN);
    gpio_set_irq_enabled(pin_val, (u32)_interrupt_states[pin_idx], true);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.attach_interrupt(pin_val, true, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.attach_interrupt(pin_val, false, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.attach_interrupt(pin_val, true, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.attach_interrupt(pin_val, false, _interrupts[pin_idx],
                             _interrupt_states[pin_idx],
                             _interrupt_args[pin_idx]);
    break;
  }
  return true;
}

bool DigitalPins::detach_interrupt(pinmap::Digital pin) {
  if (!_initialized) {
    debug::fatal(
        "DigitalPins: tried detaching interrupt when unintialized\r\n");
    return false;
  }

  u8 pin_idx = (u8)pin;
  u8 pin_val = pin_number(pin);
  _interrupts[pin_idx] = nullptr;
  _interrupt_states[pin_idx] = DigitalPinInterruptState::EDGE_FALL;
  _interrupt_args[pin_idx] = nullptr;

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_irq_enabled(pin_val, (u32)_interrupt_states[pin_idx], false);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.detach_interrupt(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.detach_interrupt(pin_val, false);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.detach_interrupt(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.detach_interrupt(pin_val, false);
    break;
  }
  return true;
}

bool DigitalPins::enable_interrupt(pinmap::Digital pin) {
  if (!_initialized) {
    debug::fatal("DigitalPins: tried enabling interrupt when unintialized\r\n");
    return false;
  }

  u8 pin_idx = (u8)pin;
  u8 pin_val = pin_number(pin);

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_irq_enabled(pin_val, (u32)_interrupt_states[pin_idx], true);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.enable_interrupt(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.enable_interrupt(pin_val, false);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.enable_interrupt(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.enable_interrupt(pin_val, false);
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
  u8 pin_val = pin_number(pin);

  switch (pin_owner(pin)) {
  case pinmap::DigitalPinOwner::PICO:
    gpio_set_irq_enabled(pin_val, (u32)_interrupt_states[pin_idx], false);
    break;
  case pinmap::DigitalPinOwner::DMUX1A:
    _dmux_1.disable_interrupt(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX1B:
    _dmux_1.disable_interrupt(pin_val, false);
    break;
  case pinmap::DigitalPinOwner::DMUX2A:
    _dmux_2.disable_interrupt(pin_val, true);
    break;
  case pinmap::DigitalPinOwner::DMUX2B:
    _dmux_2.disable_interrupt(pin_val, false);
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
