#pragma once
#include <cstdint>
#include <stdlib.h>
extern "C" {
    #include "hardware/gpio.h"

}
#include "types.hpp"
#include "pins/MCP23S17.hpp"
#define IR_SENSOR_HPP

class IRsensor {
    public:
        void init();
        void read_raw();

    private:
        
};















