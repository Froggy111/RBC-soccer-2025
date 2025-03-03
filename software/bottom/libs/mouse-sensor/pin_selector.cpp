#include "include/pin_selector.hpp"

types::u8 PinSelector::get_pin(MouseSensorPinMap pin) {
//   if (debugMode) {
//     return debug_pins[static_cast<types::u8>(pin)];
//   }

  switch (mouseSensorId) {
    case 1:
      return mouse_sensor1_pins[static_cast<types::u8>(pin)];
    case 2:
      return mouse_sensor2_pins[static_cast<types::u8>(pin)];
    default:
      return 0;
  }
}