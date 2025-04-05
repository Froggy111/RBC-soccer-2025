#pragma once

#include "types.hpp"

namespace line_sensors {

const types::u8 SENSOR_COUNT      = 48;
const types::u16 SENSOR_THRESHOLD = 1000; // calibrate!
const types::f32 EVADE_MULTIPLIER = 0.8;

const types::Vec2f32 SENSOR_VECTORS[SENSOR_COUNT] = {
    types::Vec2f32(-12, -65).normalize(),
    types::Vec2f32(-28, -71).normalize(),
    types::Vec2f32(-40, -80).normalize(),
    types::Vec2f32(-54, -91.5).normalize(),
    types::Vec2f32(-51.5, -73.5).normalize(),
    types::Vec2f32(-59, -66).normalize(),
    types::Vec2f32(-66.5, -58.5).normalize(),
    types::Vec2f32(-74, -51).normalize(),
    types::Vec2f32(-90, 50.5).normalize(),
    types::Vec2f32(-97.5, 37).normalize(),
    types::Vec2f32(-102.5, 22).normalize(),
    types::Vec2f32(-105.5, 5.5).normalize(),
    types::Vec2f32(-105.5, -5.5).normalize(),
    types::Vec2f32(-102.5, -22).normalize(),
    types::Vec2f32(-97.5, -37).normalize(),
    types::Vec2f32(-90, -50.5).normalize(),
    types::Vec2f32(56.5, 90).normalize(),
    types::Vec2f32(43, 97.5).normalize(),
    types::Vec2f32(28, 102.5).normalize(),
    types::Vec2f32(12, 105.5).normalize(),
    types::Vec2f32(-12, 105.5).normalize(),
    types::Vec2f32(-28, 102.5).normalize(),
    types::Vec2f32(-43, 97.5).normalize(),
    types::Vec2f32(-56.5, 90).normalize(),
    types::Vec2f32(-74, 51).normalize(),
    types::Vec2f32(-66.5, 58.5).normalize(),
    types::Vec2f32(-59, 66).normalize(),
    types::Vec2f32(-51.5, 73.5).normalize(),
    types::Vec2f32(51.5, 73.5).normalize(),
    types::Vec2f32(59, 66).normalize(),
    types::Vec2f32(66.5, 58.5).normalize(),
    types::Vec2f32(74, 51).normalize(),
    types::Vec2f32(90, -50.5).normalize(),
    types::Vec2f32(97.5, -37).normalize(),
    types::Vec2f32(102.5, -22).normalize(),
    types::Vec2f32(105.5, -5.5).normalize(),
    types::Vec2f32(105.5, 5.5).normalize(),
    types::Vec2f32(102.5, 22).normalize(),
    types::Vec2f32(97.5, 37).normalize(),
    types::Vec2f32(90, 50.5).normalize(),
    types::Vec2f32(74, -51).normalize(),
    types::Vec2f32(66.5, -58.5).normalize(),
    types::Vec2f32(59, -66).normalize(),
    types::Vec2f32(51.5, -73.5).normalize(),
    types::Vec2f32(54, -91.5).normalize(),
    types::Vec2f32(40, -80).normalize(),
    types::Vec2f32(28, -71).normalize(),
    types::Vec2f32(12, -65).normalize(),
};

class LineSensors {
  public:
    void init(void);
    static void data_processor(const types::u8 *data, types::u16 data_len);
    types::Vec2f32 evade_vector(void);

  private:
    static types::u16 _data[SENSOR_COUNT];
    static types::Vec2f32 _evade_vector;
};

} // namespace line_sensors
