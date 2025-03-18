#pragma once
#include <cstdint>
#include <stdlib.h>
extern "C" {
    #include <hardware/gpio.h>
} 
#include "types.hpp"
#include "pins/MCP23S17.hpp"
#define IR_SENSOR_HPP

class IRSensor {
    public:
        IRSensor(int n_samples);
        void add(bool state);
        float average(void);
        void modulation_timer_callback(struct repeating_timer *t);
        void rising_edge(uint gpio, uint32_t events);
        void falling_edge(uint gpio, uint32_t events);
        void setup();
        void loop();
    private:
        int n_samples;
        bool *samples;
        int current_idx;
}















