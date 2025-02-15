#include "pin_selector.hpp"

types::u8 PinSelector::get_pin(DriverPinMap pin) {
  if (debugMode) {
    return static_cast<types::u8>(static_cast<DriverDbgPinMap>(pin));
  }

  switch (driverId) {
  case 1:
    return static_cast<types::u8>(static_cast<Driver1PinMap>(pin));
  case 2:
    return static_cast<types::u8>(static_cast<Driver2PinMap>(pin));
  case 3:
    return static_cast<types::u8>(static_cast<Driver3PinMap>(pin));
  case 4:
    return static_cast<types::u8>(static_cast<Driver4PinMap>(pin));
  default:
    // Handle error case or return a default value
    return 0xFF;
  }
}

void PinSelector::set_debug_mode(bool mode) { debugMode = mode; }

void PinSelector::set_driver_id(types::u8 id) { driverId = id; }