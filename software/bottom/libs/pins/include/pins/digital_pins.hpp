#pragma once
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pinmap.hpp"
#include "types.hpp"
#include <pico/stdlib.h>

namespace Pins {

// NOTE: INPUT_PULLDOWN is not defined as MCP23S17 only has internal pullups, not pulldowns
// NOTE: If INPUT_PULLUP functionality is needed, inform me.
enum class DigitalPinMode : types::u8 { INPUT, OUTPUT, INPUT_PULLUP };
enum class DigitalPinInterruptState : types::u8 {
  EDGE_RISE = GPIO_IRQ_EDGE_RISE,
  EDGE_FALL = GPIO_IRQ_EDGE_FALL,
  LEVEL_HIGH = GPIO_IRQ_LEVEL_HIGH,
  LEVEL_LOW = GPIO_IRQ_LEVEL_LOW
};

using DigitalPinInterrupt = void (*)(pinmap::DigitalPins,
                                     DigitalPinInterruptState, void *);

class DigitalPins {
public:
  DigitalPins();
  /**
   * @brief initialise DMUX, GPIOs, etc.
   * @warning MUST be called before any other class functions (checks will be made)
   */
  bool init(void);
  bool read(pinmap::DigitalPins pin);
  bool write(pinmap::DigitalPins pin, bool value);
  bool change_mode(pinmap::DigitalPins pin, DigitalPinMode pin_mode);
  /**
   * @brief attach interrupt to digital pin
   * @param pin: digital pin selection
   * @param interrupt_state: interrupt state to trigger interrupt on
   * @param interrupt_handler: function to be run for this interrupt condition
   * @param args: custom user args to be passed
   */
  bool attach_interrupt(pinmap::DigitalPins pin,
                        DigitalPinInterruptState interrupt_state,
                        DigitalPinInterrupt interrupt_handler, void *args);
  bool detach_interrupt(pinmap::DigitalPins pin);
  bool enable_interrupt(pinmap::DigitalPins pin);
  bool disable_interrupt(pinmap::DigitalPins pin);

private:
};

} // namespace Pins
