#pragma once
#include <cstdint>
#include <stdlib.h>
extern "C" {
    #include <hardware/pwm.h>
    //idk it works like that for me and ajar, idk cause whenever i include libs, always shows not found??
} //yea wait why is it like that, hmmmm oh wait try refresh window lmaoooo
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















