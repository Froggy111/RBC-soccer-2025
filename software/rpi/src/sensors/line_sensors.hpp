#pragma once

#include "types.hpp"

namespace line_sensors {

const types::u8 SENSOR_COUNT = 48;

class LineSensors {
  public:
    void init(void);
    static void data_processor(const types::u8 *data, types::u16 data_len);

  private:
    static volatile int data[SENSOR_COUNT];
};

} // namespace line_sensors