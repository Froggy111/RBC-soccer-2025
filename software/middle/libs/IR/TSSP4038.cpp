#include "TSSP4038.hpp"
#include "include/TSSP4038.hpp"
extern "C" {
#include "hardware/gpio.h"
}
#include "types.hpp"
#include "comms.hpp"
#include "pinmap.hpp"


IRSensor::IRSensor(int n_samples) { 
    this->n_samples = n_samples; 
    this->samples = (bool*) malloc(n_samples * sizeof(bool)); 
    memset((void*) this->samples, 0, n_samples * sizeof(bool)); 
    this->current_idx = 0; 
    return; 
} 
  
void IRSensor::add(bool state) { 
    this->samples[this->current_idx] = state; 
    this->current_idx = (this->current_idx + 1) % n_samples; 
    return; 
} 

float IRSensor::average(void) { 
    int total = 0; 
    for (int i = 0; i < n_samples; i++) { 
        total += (int) this->samples[i]; 
        } 
    float average = (float) total / (float) n_samples; 
    return average; 
}

//timer callback for mod
static bool IRSensor::modulation_timer_callback(struct repeating_timer *t) {
    mod_step = (mod_step + 1) % 5;
    return true; //keeps on repeating, non-blocking code
}

//interrupt when rising edge detected
void IRSensor::rising_edge(uint8_t gpio, uint32_t events) { 
    for (int i = 0; i<num_ir_sensors; i++) { 
        if (gpio == ir_pins[i]) { 
            n_samples[i]->add(true); 
            rising_time[i] = time_us_32();
            return; 
        } 
    }
    return; 
}

//interrupt when falling edge detected
void IRSensor::falling_edge(uint8_t gpio, uint32_t events) { 
    for (int i = 0; i<num_ir_sensors; i++) { 
        if (gpio == ir_pins[i]) { 
            n_samples[i]->add(false); 
            falling_time[i] = time_us_32();
            return; 
        } 
    }
    return; 
}
