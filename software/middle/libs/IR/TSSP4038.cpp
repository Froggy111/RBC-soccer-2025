#include "TSSP4038.hpp"
#include "include/IR.hpp"
extern "C"{
#include "hardware/gpio.h"
#include "stdio.h"
}
#include "types.hpp"
#include "comms.hpp"
#include "pinmap.hpp"

const int freq = 40000; 
const int period = 1000000 / freq; 
const int waveform_freq = 1200; //mod frequency
const float samples_per_window = freq / waveform_freq;

volatile int mod_step = 0; //keep track of which mod step we are on
struct repeating_timer modulation_timer;

//GPIO for array of pins :D
const int ir_pins[] = {
    pinmap::Pico::IR1, pinmap::Pico::IR2, pinmap::Pico::IR3, pinmap::Pico::IR4,
    pinmap::Pico::IR5, pinmap::Pico::IR6, pinmap::Pico::IR7, pinmap::Pico::IR8,
    pinmap::Pico::IR9, pinmap::Pico::IR10, pinmap::Pico::IR11, pinmap::Pico::IR12,
    pinmap::Pico::IR13, pinmap::Pico::IR14, pinmap::Pico::IR15, pinmap::Pico::IR16,
    pinmap::Pico::IR17, pinmap::Pico::IR18, pinmap::Pico::IR19, pinmap::Pico::IR20,
    pinmap::Pico::IR21, pinmap::Pico::IR22, pinmap::Pico::IR23, pinmap::Pico::IR24
};

const int num_ir_sensors = sizeof(ir_pins) / sizeof(ir_pins[0]);

// Create array of Samples obj, each IR sensor??
Samples* ir_samples[num_ir_sensors];

//rising edge and falling edge timestamps
volatile uint32_t rising_time[num_ir_sensors] = {0};
volatile uint32_t falling_time[num_ir_sensors] = {0};

//mod steps
const float mod_duty_cycle[5] = {1.0, 0.25, 0.0625, 0.015625, 0.0}

Samples::Samples(int n_samples) { 
    this->n_samples = n_samples; 
    this->samples = (bool*) malloc(n_samples * sizeof(bool)); 
    memset((void*) this->samples, 0, n_samples * sizeof(bool)); 
    this->current_idx = 0; 
    return; 
} 
  
void Samples::add(bool state) { 
this->samples[this->current_idx] = state; 
this->current_idx = (this->current_idx + 1) % n_samples; 
return; 
} 

float Samples::average(void) { 
int total = 0; 
for (int i = 0; i < n_samples; i++) { 
    total += (int) this->samples[i]; 
    } 
float average = (float) total / (float) n_samples; 
return average; 
}

//timer callback for mod
bool modulation_timer_callback(struct repeating_timer *t) {
    mod_step = (mod_step + 1) % 5;
    return true; //keeps on repeating, non-blocking code
}

//interrupt when rising edge detected
void rising_edge(unit gpio, uint32_t events) { 
    for (int i = 0; i<num_ir_sensors; i++) { 
        if (gpio == ir_pins[i]) { 
            ir_samples[i]->add(true); 
            rising_time[i] = time_us_32();
            return; 
        } 
    }
    return; 
}

//interrupt when falling edge detected
void falling_edge(unit gpio, uint32_t events) { 
    for (int i = 0; i<num_ir_sensors; i++) { 
        if (gpio == ir_pins[i]) { 
            ir_samples[i]->add(false); 
            falling_time[i] = time_us_32();
            return; 
        } 
    }
    return; 
}

//mainloop
int main() { 
    comms::USB_CDC.printf("Initialising IR sensors...\n");

    // initialise each IR pin as input 
    for (int i = 0; i < num_ir_sensors; i++) {
        gpio_init(ir_pins[i]); // initialise the GPIO pin
        gpio_set_dir(ir_pins[i], GPIO_IN); // set the dir to input
        ir_samples[i] = new Samples(samples_per_window);  

        gpio_set_irq_enabled_with_callback(ir_pins[i], GPIO_IRQ_EDGE_RISE, true, rising_edge);
        gpio_set_irq_enabled_with_callback(ir_pins[i], GPIO_IRQ_EDGE_RISE, true, falling_edge);
        comms::USB_CDC.printf("IR Sensor %d initialized at GPIO pin %d\n", i + 1, ir_pins[i]);
        }
        //start mod timer
        add_repeating_timer_ms(-833, modulation_timer_callback, NULL, &modulation_timer); 

        while (true){
            uint64_t start_time = time_us_64();

            for (int i = 0; i < num_ir_sensors; i++) {
                bool state = gpio_get(ir_pins[i]); 
                ir_samples[i]->add(state); 
                
                // Output the state and average for each IR sensor
                comms::USB_CDC.printf(">IR%d: s: %d\n", i + 1, state);
                comms::USB_CDC.printf(">IR%d: avg: %.2f\n", i + 1, ir_samples[i]->average());
            }
            while (time_us_64() - start_time < period) { 
                continue; 
            }

        }

        return 0;

} 

/*
void loop() { 
    u_int64_t start_time = micros(); 
    // Loop through each IR sensor, read value, add to the respective Samples object
    for (int i = 0; i < num_ir_sensors; i++) {
        bool state = gpio_get(ir_pins[i]); 
        ir_samples[i]->add(state); 
        
        // Output the state and average for each IR sensor
        comms::USB_CDC.printf(">IR%d: s: %d\n", i + 1, state);
        comms::USB_CDC.printf(">IR%d: avg: %.2f\n", i + 1, ir_samples[i]->average());
    }

    // Wait until the next period
    while (micros() - start_time < period) { continue; } 
}

*/




