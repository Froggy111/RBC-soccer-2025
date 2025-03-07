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
    
    //init dmux
    dmux.init(1, spi_obj);

    //init dmux gpio pins
    dmux.init_gpio((int) pinmap::Mux1A::AMUX_S0, true, true);
    dmux.init_gpio((int) pinmap::Mux1A::AMUX_S1, true, true);
    dmux.init_gpio((int) pinmap::Mux1A::AMUX_S2, true, true);
    dmux.init_gpio((int) pinmap::Mux1A::AMUX_S3, true, true);
    dmux.init_gpio((int) pinmap::Mux1A::AMUX_EN, true, true);

    //init adc
    adc_init();  // initialise ADC 
    adc_gpio_init((int) pinmap::Pico::AMUX1_COM); 
    adc_gpio_init((int) pinmap::Pico::AMUX2_COM); 
    adc_gpio_init((int) pinmap::Pico::AMUX3_COM);
}

void LineSensor::select_channel(uint8_t channel) {
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