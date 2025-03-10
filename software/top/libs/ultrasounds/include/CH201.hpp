#include <cstdint>
extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
#include <vector>
}

class Ultrasound {

private:
    ch_dev_t ch201_sensor;

public:
    void init(uint8_t id, uint8_t baudrate);

    void set_max_range(uint8_t dist);

    void set_mode(ch_mode_t req_mode);

    uint32_t get_dist(ch_range_t req_range);
}