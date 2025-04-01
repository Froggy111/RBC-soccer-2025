#include "types.hpp"

namespace IR {

const types::u8 SENSOR_COUNT = 24;
struct ModulationData {
    types::u32 uptime = 0;
    inline void reset(void) volatile { uptime = 0; }
};
static volatile ModulationData modulation_data[SENSOR_COUNT];
const double MODULATION_FREQ = 1200; // 833.333us per cycle
const double PULSE_FREQ      = 40000;

} // namespace IR
