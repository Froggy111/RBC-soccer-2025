#pragma once
#include "pinmap.hpp"
#include "pins.hpp"
#include "pico/stdio.h"

class MotorDriver
{
public:
    void init(int32_t SPI_SPEED);

private:
    void init_spi(int32_t SPI_SPEED);
    void init_pins();

    PinInputControl inputControl;
    PinOutputControl outputControl;
};