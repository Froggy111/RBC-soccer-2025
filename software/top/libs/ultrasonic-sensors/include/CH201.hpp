#include <cstdint>
extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
#include <vector>
}

class UltrasoundSensor{

private:
    ch_dev_t ch201_sensor;

public:
    UltrasoundSensor(void) = default;

    void init(uint8_t id, uint8_t baudrate);

    void set_max_range(uint8_t dist);

    void set_mode(ch_mode_t req_mode);

    uint32_t get_dist(ch_range_t req_range);
}

class UltrasoundGroup{
private:
    ch_group_t ch201_group;
    
    UltrasoundSensor sensors[16];

    uint32_t reading_buffer[16];

public:
    UltrasoundGroup(void) = default;

    void init(uint8_t id);

    void add_sensor(uint_8 id);

    std::pair<int32_t, int32_t> approx_pos(); //May not be used but prolly useful in triangulating position

    void get_group_readings();

}