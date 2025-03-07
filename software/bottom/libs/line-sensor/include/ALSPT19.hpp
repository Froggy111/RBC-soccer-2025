#pragma once
#include "pinmap.hpp"
extern "C" {
    #include "hardware/adc.h"
    #include "hardware/gpio.h"

}
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
        
};
















