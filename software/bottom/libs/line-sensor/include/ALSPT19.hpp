#pragma once
#include "types.hpp"
#include "pins/MCP23S17.hpp"

class LineSensor {
    public:
        void init();
        uint16_t read_raw(uint8_t line_sensor_id);

    private:
      void select_channel(uint8_t channel);
};
















