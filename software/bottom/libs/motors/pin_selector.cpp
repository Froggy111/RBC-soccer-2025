#include "pin_selector.hpp"

types::u8 PinSelector::get_pin(DriverPinMap pin) {
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

void PinSelector::set_debug_mode(bool mode) { debugMode = mode; }

void PinSelector::set_driver_id(types::u8 id) { driverId = id; }