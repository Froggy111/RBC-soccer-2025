#include "line_sensors.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include <memory.h>

using namespace types;
namespace line_sensors {

Vec2f32 LineSensors::_evade_vector   = Vec2f32(0, 0);
u16 LineSensors::_data[SENSOR_COUNT] = {0};

void LineSensors::init() {
    debug::info("init line sensors");
    comms::USB_CDC.registerBottomPicoHandler(
        comms::RecvBottomPicoIdentifiers::LINE_SENSOR_DATA, data_processor);
    debug::info("inited line sensors");
    return;
}

void LineSensors::data_processor(const u8 *data, u16 data_len) {
    debug::info("line sensors data recieved");
    memcpy((void *)_data, data, data_len);
    u8 activated_count    = 0;
    Vec2f32 summed_vector = Vec2f32(0, 0);
    for (int i = 0; i < SENSOR_COUNT; i++) {
        debug::info("Line sensor %d reads %u", i, _data[i]);
        if (_data[i] > SENSOR_THRESHOLD) {
            debug::info("Line sensor %d over threshold", i);
            activated_count += 1;
            summed_vector = summed_vector + SENSOR_VECTORS[i];
        }
    }
    _evade_vector = summed_vector / activated_count * EVADE_MULTIPLIER;
    debug::info("New evade vector: %f, %f", _evade_vector.x, _evade_vector.y);
    return;
}

Vec2f32 LineSensors::evade_vector(void) { return _evade_vector; }

} // namespace line_sensors
