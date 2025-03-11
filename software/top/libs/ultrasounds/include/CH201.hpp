extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
#include "hardware/i2c.h"
}
#include "pins/MCP23S17.hpp"

class Ultrasound {

private:
  ch_dev_t ch201_sensor;
  int device_id;
  i2c_inst_t *i2c;

  static MCP23S17* dmux1, dmux2;

public:
    static void group_init();
    void init(uint8_t id, uint8_t baudrate);

    void set_max_range(uint8_t dist);

    void set_mode(ch_mode_t req_mode);

    uint32_t get_dist(ch_range_t req_range);
}