#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include <tuple>
#include <vector>
#include <cmath>

namespace IR {

const types::u8 SENSOR_COUNT = 24;
struct ModulationData {
    int uptime = 0;
    inline void reset(void) volatile { uptime = 0; }
};
const double MODULATION_FREQ = 1200; // 833.333us per cycle
const double PULSE_FREQ      = 40000;

enum ModulationTimeMappings{
    UNDETECTED = 0,
    FIRST = 100,
    SECOND = 150,
    THIRD = 200
};

class IR {
    public:
    void init(void);

    void data_processor(const int *data, int data_len); 

    std::tuple<float, float> find_ball();

    float normalize_angle(float angle);

    private:
    static volatile ModulationData modulation_buffer[SENSOR_COUNT];
    int modulation_data[SENSOR_COUNT];
    float ir_headings[SENSOR_COUNT] = {-97.5, -112.5, -127.5, -142.5, -157.5, -172.5, 172.5, 157.5, 142.5, 127.5, 112.5, 97.5, 82.5, 67.5, 52.5, 37.5, 22.5, 7.5, -7.5, -22.5, -37.5, -52.5, -67.5, -82.5};
};

extern IR IR_sensors;

} // namespace IR
    