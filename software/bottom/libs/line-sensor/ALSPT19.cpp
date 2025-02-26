#include "ALSPT19.hpp"
extern "C" {
    #include "types.hpp"
    #include "hardware/adc.h"
    #include <pico/stdio.h>
    #include <pico/stdlib.h>

}

void LineSensor::init(types::u8 id) {
    printf("---> Initializing ALSPT19\n");

    if (id == (types::u8)-1) {
        pinSelector.set_debug_mode(true);
    } else {
        pinSelector.set_driver_id(id);
    }

    adc_init();  // initialize ADC 
    adc_gpio_init(sensor_pin);  // Configure GPIO 
}

uint16_t LineSensor::read_raw() {
    adc_select_input(sensor_pin);
    return adc_read(); //read adc value 
}

float LineSensor::read_voltage() {
    uint16_t raw_value = read_raw();
    float voltage = (raw_value * 3.3f) / 4095.0f;
    return voltage;
}

void LineSensor::read() {
    float voltage = read_voltage();
    printf("Sensor Voltage: %.3fV\n", voltage);
}
