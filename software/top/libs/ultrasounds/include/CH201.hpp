extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
}

class Ultrasound {

private:
  int id;
  ch_dev_t ch201_sensor;
  static ch_group_t ch201_group;

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

  static void sensor_int_callback(ch_group_t *grp_ptr, uint8_t io_index);

public:
  /**
   * @brief Group init, called first to initialize the ch201 group.
   * 
   */
  static void group_init();

  /**
   * @brief Init the dmux and the CH201 sensor, called after group_init and before group_start.
   * 
   * @param us_id (1-indexed)
   */
   void init(int us_id);

  /**
   * @brief Group start, called last to start the ch201 group.
   * 
   */
  static void group_start();

  /**
   * @brief Reset the CH201
   * 
   */
  void reset();
};