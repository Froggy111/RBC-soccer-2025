extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
#include "hardware/i2c.h"
}
#include "pins/MCP23S17.hpp"

class Ultrasound {

private:
  ch_dev_t ch201_sensor;
  int id;
  i2c_inst_t *i2c;

  // group vars
  static ch_group_t ch201_group;
  static MCP23S17 *dmux1, *dmux2;

  /**
   * @brief Set the prog pin based on the device ID
   * 
   * @param prog 
   * @param init 
   */
  void set_prog(uint8_t prog, bool init = false);

  /**
   * @brief Set the int pin based on the device ID
   * 
   * @param int_val 
   * @param init 
   */
  void set_int(uint8_t int_val, bool init = false);

public:
  /**
   * @brief Group init, meant to be called before init.
   * 
   */
  static void group_init();
  static void group_start();

  /**
   * @brief Init the dmux and the CH201 sensor
   * 
   * @param i2c_inst 
   * @param us_id (1-indexed)
   */
  void init(i2c_inst_t *i2c_inst, int us_id);

  /**
   * @brief Reset the CH201
   * 
   */
  void reset();

  /**
   * @brief Get the distance that the ch201 senses
   * 
   * @param req_range 
   * @return uint32_t 
   */
  uint32_t get_dist(ch_range_t req_range);
};