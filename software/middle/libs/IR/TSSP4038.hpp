#pragma once
#include <cstdint>
#include <stdlib.h>
extern "C" {
    #include "hardware/gpio.h"

}
#include "types.hpp"

class IRsensor {
    public:
        IRsensor();
        bool begin();
        void read();
        void init(types::u8 id);
        bool is_signal_detected();
        void read_signal();
        void start_reading();
        void stop_reading();
        void print_reading();
    private:
        bool _last_state();
        bool _current_state();
        uint32_t _raw_readings(200);
        uint _reading_count
        uint8_t _pin;
};















