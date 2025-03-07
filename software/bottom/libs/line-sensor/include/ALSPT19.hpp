#pragma once
#include "pinmap.hpp"
#include <cstdint>
extern "C" {
    #include "hardware/adc.h"
    #include "hardware/gpio.h"

}
#include "types.hpp"
#include "pins/MCP23S17.hpp"
#define LINE_SENSOR_HPP

class LineSensor {
    public:
        void init(types::u8 id, spi_inst_t *spi_obj);
        uint16_t read_raw(uint8_t line_sensor_id);
        float read_voltage();

    private:
        void select_channel(uint8_t channel);
        MCP23S17 dmux;
};
















