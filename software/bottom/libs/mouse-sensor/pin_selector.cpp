#include "include/pin_selector.hpp"

types::u8 Pins::get_pin(MouseSensorPinMap pin) {
  switch (mouseSensorId) {
    case 1:
      return mouse_sensor1_pins[static_cast<types::u8>(pin)];
    case 2:
      return mouse_sensor2_pins[static_cast<types::u8>(pin)];
    default:
      return mouse_sensor1_pins[static_cast<types::u8>(pin)];
  }
}

void Pins::set_mouse_sensor_id(types::u8 id) {
  mouseSensorId = id;
}