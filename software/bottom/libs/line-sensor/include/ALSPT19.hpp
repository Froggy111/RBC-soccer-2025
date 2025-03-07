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

        void init(types::u8 id);
        uint16_t read_raw();
        float read_voltage();
    private:
        void select_channel(uint8_t channel);
        uint16_t get_raw();
        uint16_t get_integration_time();
        uint8_t get_gain();
        void write_register(uint8_t reg, uint8_t val);
        uint8_t read_register(uint8_t reg);
        PinSelector pinSelector;
        uint8_t comm_pin;
};
















