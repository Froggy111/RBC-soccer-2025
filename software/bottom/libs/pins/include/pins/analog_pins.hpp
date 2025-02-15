#pragma once

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pinmap.hpp"
#include "types.hpp"
#include "pins/digital_pins.hpp"
#include <pico/stdlib.h>

namespace Pins {

enum class AnalogPinInterruptState : types::u8 { LEVEL_HIGH, LEVEL_LOW };
using AnalogPinInterrupt = void (*)(pinmap::AnalogPins, AnalogPinInterruptState,
                                    void *);

/**
 * @brief Wrapper around all analog pins for convenience
 * @warning This depends on DigitalPins. DigitalPins MUST be initialised before this.
 */
class AnalogPins {
public:
  /**
   * @brief constructs AnalogPins object
   * @param digital_pins: digital pins object to use for interfacing digital pins needed
   * @param i2c_iface: i2c interface to use for interfacing external ADC. Should be initialised externally.
   */
  AnalogPins(DigitalPins *digital_pins, i2c_inst *i2c_iface);
  /**
   * @brief initialise ADCs, etc.
   * @warning MUST be called before any other class functions (checks will be made)
   * @warning DigitalPins.init() MUST be called before this. (checks will be made)
   */
  bool init(void);
  types::f32 read(pinmap::AnalogPins pin);
  /**
   * @brief attach interrupt to an external ADC pin
   * @param pin: analog pin selection
   * @param high_limit: high threshold to trigger interrupt above this limit
   * @param low_limit: low threshold to trigger interrupt below this limit (set to NaN to disable, and use traditional comparator mode only)
   * @param interrupt_handler: function to run for this interrupt condition.
   * @param args: custom user args to be passed 
   */
  bool attach_interrupt(pinmap::AnalogPins pin, types::f32 high_limit,
                        types::f32 low_limit,
                        AnalogPinInterrupt interrupt_handler, void *args);
  bool detach_interrupt(pinmap::AnalogPins pin);
  bool enable_interrupt(pinmap::AnalogPins pin);
  bool disable_interrupt(pinmap::AnalogPins pin);
};

} // namespace Pins
