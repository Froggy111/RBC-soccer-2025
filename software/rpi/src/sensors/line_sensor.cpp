#pragma once

#include "comms.hpp"
#include "debug.hpp"
#include "line_sensors.hpp"
#include "types.hpp"
#include <memory.h>

using namespace types;
namespace line_sensors {

Vec2f32 LineSensors::_evade_vector   = Vec2f32(0, 0);
u16 LineSensors::_data[SENSOR_COUNT] = {0};

void LineSensors::init() {
    comms::USB_CDC.registerBottomPicoHandler(
        comms::RecvBottomPicoIdentifiers::LINE_SENSOR_DATA, data_processor);
}

void LineSensors::data_processor(const u8 *data, u16 data_len) {
    memcpy((void *)_data, data, data_len);
    u8 activated_count    = 0;
    Vec2f32 summed_vector = Vec2f32(0, 0);
    for (int i = 0; i < SENSOR_COUNT; i++) {
        if (_data[i] > SENSOR_THRESHOLD) {
            activated_count += 1;
            summed_vector = summed_vector + SENSOR_VECTORS[i];
        }
    }
    _evade_vector = summed_vector / activated_count * EVADE_MULTIPLIER;
}

Vec2f32 LineSensors::evade_vector(void) { return _evade_vector; }

} // namespace line_sensors
