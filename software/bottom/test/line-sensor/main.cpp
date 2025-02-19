#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

//ADC pins
#define line_sensor_pin 26
#define line_sensor_pin2 27
#define line_sensor_pin3 28
#define line_sensor_pin4 29

//PWM pins???

#define threshold // insert value later

void setup_sensor(){
    adc_init();
    adc_gpio_init(line_sensor_pin);
    adc_gpio_init(line_sensor_pin2);
    adc_gpio_init(line_sensor_pin3);
    adc_gpio_init(line_sensor_pin4);
}

uint16_t read_adc(uint8_t adc_channel) {
    adc_select_input(adc_channel);
    return adc_read(); //12 bit value which ranges from 0-4095
} 

//main algo for avoidance
void check_lines() {
    uint16_t sensor1_value = read_adc(0);
    uint16_t sensor2_value = read_adc(1);
    uint16_t sensor3_value = read_adc(2);
    uint16_t sensor4_value = read_adc(3);

    if (sensor1_value > threshold) {
        //do sth
    }
    else if (sensor2_value > threshold) {
        //do sth
    }
    else if (sensor3_value > threshold) {
        //do sth
    }
    else if (sensor4_value > threshold) {
        //do sth
    }
    else {
        //do sth
    }
}




