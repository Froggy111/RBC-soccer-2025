#pragma once
#include <cstdint>
#include <stdlib.h>
extern "C" {
    #include <hardware/gpio.h>
} 
#include "types.hpp"
#include "pins/MCP23S17.hpp"
#define IR_SENSOR_HPP

const int ir_pins[];
const int num_ir_sensors;
const int freq;
const int period;
const int waveform_freq;
const float samples_per_window;
volatile int mod_step = 0;
struct repeating_timer modulation_timer;
Samples* ir_samples[num_ir_sensors];


class IRSensor {
    public:
        IRSensor(int n_samples);
        void add(bool state);
        float average(void);
        static bool modulation_timer_callback(struct repeating_timer *t);
        static void rising_edge(uint gpio, uint32_t events);
        static void falling_edge(uint gpio, uint32_t events);
        void init();
        void read_raw();
    private:
        int n_samples;
        bool *samples;
        int current_idx;
}















