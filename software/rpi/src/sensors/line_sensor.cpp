#pragma once

#include "comms.hpp"
#include "debug.hpp"
#include "line_sensors.hpp"
#include "types.hpp"

using namespace types;
namespace line_sensors {

void LineSensors::init() {
    comms::USB_CDC.registerBottomPicoHandler(
        comms::RecvBottomPicoIdentifiers::LINE_SENSOR_DATA, data_processor);
}

void LineSensors::data_processor(const u8 *data, u16 data_len) {}

} // namespace line_sensors
