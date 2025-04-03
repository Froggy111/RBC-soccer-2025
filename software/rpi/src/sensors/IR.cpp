#include "IR.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include <cstring>

using namespace types;

namespace IR {

volatile ModulationData IR::modulation_data[SENSOR_COUNT] = {ModulationData()};

void IR::init(void) {
    comms::USB_CDC.registerMiddlePicoHandler(
        comms::RecvMiddlePicoIdentifiers::IR_DATA, data_processor);
}

void IR::data_processor(const types::u8 *data, types::u16 data_len) {
    memcpy((void *)modulation_data, data, data_len);
    for (u8 i = 0; i < SENSOR_COUNT; i++) {
        debug::info("IR sensor %u uptime: %u", i, modulation_data[i].uptime);
    }
    return;
}

IR IR_sensors = IR();

} // namespace IR
