#include "pin_selector.hpp"

void DMUX_SELECTOR::select_dmux_pin(PIN_TYPE pin_type, int device_id,
                                    int &dmux_id, int &dmux_gpio_pin_no,
                                    bool &dmux_on_A) {
  // 4 hard-coded edge cases
  // cuz JIAYIIIII >:( WHY
  if (pin_type == PIN_TYPE::PROG && device_id == 5) {
    dmux_id = 1;
    dmux_gpio_pin_no = 0;
    dmux_on_A = false;
  } else if (pin_type == PIN_TYPE::INT && device_id == 5) {
    dmux_id = 1;
    dmux_gpio_pin_no = 1;
    dmux_on_A = false;
  } else if (pin_type == PIN_TYPE::PROG && device_id == 13) {
    dmux_id = 2;
    dmux_gpio_pin_no = 0;
    dmux_on_A = false;
  } else if (pin_type == PIN_TYPE::INT && device_id == 13) {
    dmux_id = 2;
    dmux_gpio_pin_no = 1;
    dmux_on_A = false;
  } else {
    // for all other configurations
    dmux_id = device_id > 8 ? 2 : 1;
    dmux_gpio_pin_no = ((device_id - 1) * 2 + (pin_type == PIN_TYPE::PROG)) % 8;
    dmux_on_A = (device_id <= 4 && device_id >= 1) || (device_id >= 9 && device_id <= 12);
  }
}