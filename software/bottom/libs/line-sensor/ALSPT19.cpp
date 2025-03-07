#include "ALSPT19.hpp"
#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"

extern "C" {
    #include "hardware/adc.h"
    #include "hardware/gpio.h"
    #include <pico/stdlib.h>
}

void LineSensor::init(types::u8 id, spi_inst_t *spi_obj) {
    printf("---> Initializing ALSPT19\n");
    
    dmux.init(1, spi_obj);

    adc_init();  // initialise ADC 
    adc_gpio_init((int) pinmap::DigitalPins::AMUX_S2); // initialise ADC pin
    adc_gpio_init((int) pinmap::DigitalPins::AMUX_S1); // initialise ADC pin
    adc_gpio_init((int) pinmap::DigitalPins::AMUX_S3); // initialise ADC pin
    
    
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