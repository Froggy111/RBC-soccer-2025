#pragma once

namespace DMUX_SELECTOR {

enum PIN_TYPE {
  PROG = 0,
  INT = 1,
};

/**
 * @brief get the dmux_id, the pin number and whether ur on bus A or B of the dmux based on the pin type and device id.
 * 
 * @param pin_type 
 * @param device_id 1-indexed
 * @param dmux_id 
 * @param dmux_gpio_pin_no 
 * @param dmux_on_A 
 */
void select_dmux_pin(PIN_TYPE pin_type, int device_id, int &dmux_id, int &dmux_gpio_pin_no,
                     bool &dmux_on_A);

}