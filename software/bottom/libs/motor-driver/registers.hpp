#pragma once

#include "pico/stdlib.h"
#include "pinmap.hpp"
#include "hardware/spi.h"

class Registers {
public:
    static uint8_t readRegister(uint8_t reg);
    static void writeRegister(uint8_t reg, uint8_t data);
};