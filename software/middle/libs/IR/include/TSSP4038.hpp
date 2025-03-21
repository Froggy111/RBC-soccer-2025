#pragma once

#include <stdlib.h>
extern "C" {
    #include <hardware/gpio.h>
    #include "hardware/timer.h"
    #include "pico/time.h"
} 
#include "types.hpp"
#include "pinmap.hpp"
#define IR_SENSORS_COUNT 24

const int ir_pins[IR_SENSORS_COUNT] = {
    (int)pinmap::Pico::IR1, (int)pinmap::Pico::IR2, (int)pinmap::Pico::IR3, (int)pinmap::Pico::IR4,
    (int)pinmap::Pico::IR5, (int)pinmap::Pico::IR6, (int)pinmap::Pico::IR7, (int)pinmap::Pico::IR8,
    (int)pinmap::Pico::IR9, (int)pinmap::Pico::IR10, (int)pinmap::Pico::IR11, (int)pinmap::Pico::IR12,
    (int)pinmap::Pico::IR13, (int)pinmap::Pico::IR14, (int)pinmap::Pico::IR15, (int)pinmap::Pico::IR16,
    (int)pinmap::Pico::IR17, (int)pinmap::Pico::IR18, (int)pinmap::Pico::IR19, (int)pinmap::Pico::IR20,
    (int)pinmap::Pico::IR21, (int)pinmap::Pico::IR22, (int)pinmap::Pico::IR23, (int)pinmap::Pico::IR24
};

//mod steps
const int mod_duty_cycle[5] = {1, 4, 16, 64, 0};

class IRSensor {
    public:
        IRSensor(int n_samples);

        float average(void);
        static bool modulation_timer_callback(struct repeating_timer *t);
        static void rising_edge(uint8_t gpio, uint32_t events);
        static void falling_edge(uint8_t gpio, uint32_t events);
        void init();

        static IRSensor* ir_samples[num_ir_sensors];
        static int mod_step;
        static struct repeating_timer modulation_timer;
    private:
        int n_samples;
        bool *samples;
        int current_idx;
};

