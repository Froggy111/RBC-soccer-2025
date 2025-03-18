#pragma once
#include <cstdint>
#include <stdlib.h>
extern "C" {
    #include <hardware/gpio.h>
} 
#include "types.hpp"
#include "pinmap.hpp"
#define IR_SENSOR_HPP

const int freq = 40000; 
const int period = 1000000 / freq; 
const int waveform_freq = 1200; //mod frequency
const float samples_per_window = freq / waveform_freq;

volatile int mod_step = 0; //keep track of which mod step we are on
struct repeating_timer modulation_timer;

const int num_ir_sensors = sizeof(ir_pins) / sizeof(ir_pins[0]);

// Create array of Samples obj, each IR sensor??
IRSensor* ir_samples[num_ir_sensors];

//rising edge and falling edge timestamps
volatile uint32_t rising_time[num_ir_sensors] = {0};
volatile uint32_t falling_time[num_ir_sensors] = {0};

//mod steps
const float mod_duty_cycle[5] = {1.0, 0.25, 0.0625, 0.015625, 0.0};

const int ir_pins[] = {
    (int)pinmap::Pico::IR1, (int)pinmap::Pico::IR2, (int)pinmap::Pico::IR3, (int)pinmap::Pico::IR4,
    (int)pinmap::Pico::IR5, (int)pinmap::Pico::IR6, (int)pinmap::Pico::IR7, (int)pinmap::Pico::IR8,
    (int)pinmap::Pico::IR9, (int)pinmap::Pico::IR10, (int)pinmap::Pico::IR11, (int)pinmap::Pico::IR12,
    (int)pinmap::Pico::IR13, (int)pinmap::Pico::IR14, (int)pinmap::Pico::IR15, (int)pinmap::Pico::IR16,
    (int)pinmap::Pico::IR17, (int)pinmap::Pico::IR18, (int)pinmap::Pico::IR19, (int)pinmap::Pico::IR20,
    (int)pinmap::Pico::IR21, (int)pinmap::Pico::IR22, (int)pinmap::Pico::IR23, (int)pinmap::Pico::IR24
};

class IRSensor {
    public:
        IRSensor(int n_samples);
        void add(bool state);
        float average(void);
        static bool modulation_timer_callback(struct repeating_timer *t);
        void rising_edge(uint8_t gpio, uint32_t events);
        void falling_edge(uint8_t gpio, uint32_t events);
        void init();
        void read_raw();
    private:
        int n_samples;
        bool *samples;
        int current_idx;
}















