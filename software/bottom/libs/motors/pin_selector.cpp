#include "pin_selector.hpp"
#include <map>
#include <utility>

extern "C" {
#include <pico/stdlib.h>
}

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

bool PinSelector::get_On_A(DriverPinMap pin) {
  if (debugMode) {
    return false;
  }

  switch (driverId) {
  case 1:
    return driver1_mux_addr[static_cast<types::u8>(pin)][1];
  case 2:
    return false;
  case 3:
    return false;
  case 4:
    return false;
  default:
    return false;
  }
}

bool PinSelector::get_On_Board1(DriverPinMap pin) {
  if (debugMode) {
    return false;
  }

  switch (driverId) {
  case 1:
    return driver1_mux_addr[static_cast<types::u8>(pin)][0];
  case 2:
    return false;
  case 3:
    return false;
  case 4:
    return false;
  default:
    return false;
  }
}

void PinSelector::set_debug_mode(bool mode) { debugMode = mode; }

void PinSelector::set_driver_id(types::u8 id) { driverId = id; }