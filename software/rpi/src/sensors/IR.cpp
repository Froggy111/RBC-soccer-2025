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
    return;
}

types::u32 IR::get_data_for_sensor_id(int id) {
    if (id < 0 || id >= SENSOR_COUNT) {
        debug::error("IR::get_data_for_sensor_id: Invalid sensor ID");
        return 0;
    }
    return modulation_data[id].uptime;
}

IR IR_sensors = IR();

} // namespace IR
