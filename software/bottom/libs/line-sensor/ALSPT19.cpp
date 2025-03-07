#include "ALSPT19.hpp"
#include "MCP23S17.hpp"

extern "C" {
    #include "types.hpp"
    #include "hardware/adc.h"
    #include "hardware/gpio.h"
    #include <pico/stdio.h>
    #include <pico/stdlib.h>
    #include <pico/stdio.h>
    #include <stdio.h>
}
LineSensor::LineSensor(uint8_t comm_pin)
    : comm_pin(comm_pin) {} //pins 0-3 for output, comm pin is connected to ADC input


void LineSensor::init(types::u8 id) {
    printf("---> Initializing ALSPT19\n");
    
    if (id == (types::u8)-1) {
        pinSelector.set_debug_mode(true);
    } else {
        pinSelector.set_driver_id(id);
    }
    MCP23S17.init(types::u8 1, spi1);

    adc_init();  // initialise ADC 
    adc_gpio_init(comm_pin);  //Configures the amux com output pin as ADC input
}

void LineSensor::select_channel(uint8_t channel) {
    MCP23S17.init_gpio(GPA7, true, true);
    MCP23S17.init_gpio(GPA6, true, true);
    MCP23S17.init_gpio(GPA4, true, true);
    MCP23S17.init_gpio(GPA3, true, true);
    MCP23S17.write_gpio(GPA7, true, true);
    MCP23S17.write_gpio(GPA6, true, true);
    MCP23S17.write_gpio(GPA5, true, true);
    MCP23S17.write_gpio(s3_pin, true, true); //selects which amux channel to read from
}

uint16_t LineSensor::read_raw() {
    adc_select_input(comm_pin); // select adc pin (28) on pico to read amux output
    return adc_read(); //read adc value
}

float LineSensor::read_voltage() {
    uint16_t raw_value = read_raw();
    float voltage = (raw_value * 3.3f) / 4095.0f;
    return voltage; //converts adc to voltage
}

void LineSensor::read() {
    float voltage = read_voltage();
    printf("Sensor Voltage: %.3fV\n", voltage);
}