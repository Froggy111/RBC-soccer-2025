#pragma once
#include "pinmap.hpp"
extern "C" {
    #include "hardware/adc.h"
    #include "hardware/gpio.h"

}
#include "pin_selector.hpp"
#include "types.hpp"
#include "MCP23S17.hpp"
#define LINE_SENSOR_HPP

class LineSensor {
    public:
        LineSensor(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t comm_pin);
        bool begin();
        void read();
        void init(types::u8 id);
        uint16_t read_raw();
        float read_voltage();
        void set_integration_time(uint16_t time);
        void set_gain(uint8_t gain);
    private:
        void select_channel(uint8_t channel);
        uint16_t get_lux();
        uint16_t get_raw();
        uint16_t get_integration_time();
        uint8_t get_gain();
        void write_register(uint8_t reg, uint8_t val);
        uint8_t read_register(uint8_t reg);
        PinSelector pinSelector;
        uint8_t comm_pin = 28;
        uint8_t s0_pin, s1_pin, s2_pin, s3_pin;
};
















