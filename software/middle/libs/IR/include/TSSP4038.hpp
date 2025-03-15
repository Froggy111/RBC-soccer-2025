#pragma once
#include <cstdint>
#include <stdlib.h>
extern "C" {
    #include "hardware/gpio.h"

}
#include "types.hpp"
#include "pins/MCP23S17.hpp"
#define IR_SENSOR_HPP

class Samples {
    Samples(int n_samples);
    void add(bool state);
    float average(void);
    void setup();
    void loop();
}















