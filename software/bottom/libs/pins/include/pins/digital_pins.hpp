#pragma once

#include "pinmap.hpp"
#include "types.hpp"
#include "pins/types.hpp"

extern "C" {
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <pico/stdlib.h>
}

namespace pins {

class DigitalPins {
public:
  DigitalPins();
  /**
   * @brief initialise DMUX, GPIOs, etc.
   * @warning MUST be called before any other class functions (checks will be made)
   */
  bool init(void);
  bool read(pinmap::Digital pin);
  bool write(pinmap::Digital pin, bool value);
  bool change_mode(pinmap::Digital pin, DigitalPinMode pin_mode);
  /**
   * @brief attach interrupt to digital pin
   * @param pin: digital pin selection
   * @param interrupt_state: interrupt state to trigger interrupt on
   * @param interrupt_handler: function to be run for this interrupt condition
   * @param args: custom user args to be passed
   */
  bool attach_interrupt(pinmap::Digital pin,
                        DigitalPinInterruptState interrupt_state,
                        DigitalPinInterrupt interrupt_handler, void *args);
  bool detach_interrupt(pinmap::Digital pin);
  bool enable_interrupt(pinmap::Digital pin);
  bool disable_interrupt(pinmap::Digital pin);

private:
};

} // namespace pins
