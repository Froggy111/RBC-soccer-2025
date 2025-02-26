#pragma once
#include <cstdint>
extern "C" {
    #include "hardware/adc.h"

}
#include "pin_selector.hpp"
#include "types.hpp"
#define LINE_SENSOR_HPP

class LineSensor {
    public:
        LineSensor();
        bool begin();
        void read();
        void init(types::u8 id);
        uint16_t read_raw();
        float read_voltage();
        void set_integration_time(uint16_t time);
        void set_gain(uint8_t gain);
    private:
        uint16_t get_lux();
        uint16_t get_raw();
        uint16_t get_integration_time();
        uint8_t get_gain();

        void write_register(uint8_t reg, uint8_t val);
        uint8_t read_register(uint8_t reg);
        PinSelector pinSelector;
        uint8_t sensor_pin = 20;
};















