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
LineSensor::LineSensor(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t comm_pin)
    : s0_pin(s0), s1_pin(s1), s2_pin(s2), s3_pin(s3), comm_pin(comm_pin) {} //pins 0-3 for output, comm pin is connected to ADC input


void LineSensor::init(types::u8 id) {
    printf("---> Initializing ALSPT19\n");

    if (id == (types::u8)-1) {
        pinSelector.set_debug_mode(true);
    } else {
        pinSelector.set_driver_id(id);
    }
    MCP23S17.init(types::u8 1, spi1);
    /* 
    gpio_init(uint8_t 28, bool GPA7, bool is_output) //DMUX1
    gpio_init(uint8_t 27, bool GPA6, bool is_output)
    gpio_init(uint8_t 25, bool GPA4, bool is_output)
    gpio_init(uint8_t 24, bool GPA3, bool is_output)
    gpio_init(s0_pin);
    gpio_init(s1_pin);
    gpio_init(s2_pin);
    gpio_init(s3_pin); 
    gpio_set_dir(s0_pin, GPIO_OUT);
    gpio_set_dir(s1_pin, GPIO_OUT);
    gpio_set_dir(s2_pin, GPIO_OUT);
    gpio_set_dir(s3_pin, GPIO_OUT); //intialise pins as output
    */
    adc_init();  // initialise ADC 
    adc_gpio_init(comm_pin);  //Configures the amux com output pin as ADC input
}

void LineSensor::select_channel(uint_8t channel) {
    MCP23S17.init_gpio(s0_pin, true, true);
    MCP23S17.init_gpio(s1_pin, true, true);
    MCP23S17.init_gpio(s2_pin, true, true);
    MCP23S17.init_gpio(s3_pin, true, true);

    MCP23S17.write_gpio(s0_pin, true, (channel & 0x01));
    MCP23S17.write_gpio(s1_pin, true, ((channel>>1) & 0x01));
    MCP23S17.write_gpio(s2_pin, true, ((channel>>2) & 0x01));
    MCP23S17.write_gpio(s3_pin, true, ((channel>>3) & 0x01)); //selects which amux channel to read from
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