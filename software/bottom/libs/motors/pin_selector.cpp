#include "pin_selector.hpp"

extern "C" {
#include <pico/stdlib.h>
}

namespace driver {
types::u8 Pins::get_pin(DriverPinMap pin) {
  if (debugMode) {
    return debug_pins[static_cast<types::u8>(pin)];
  }

  switch (driverId) {
  case 1:
    return driver1_pins[static_cast<types::u8>(pin)];
  case 2:
    return driver2_pins[static_cast<types::u8>(pin)];
  case 3:
    return driver3_pins[static_cast<types::u8>(pin)];
  case 4:
    return driver4_pins[static_cast<types::u8>(pin)];
  default:
    return 0;
  }
}

void Pins::set_debug_mode(bool mode) { debugMode = mode; }

void Pins::set_driver_id(types::u8 id) { driverId = id; }
}