#pragma once
#include "pinmap.hpp"
#include "pins.hpp"
#include "pico/stdio.h"

class MotorDriver
{
public:
    void init(int32_t SPI_SPEED);
    void command(uint32_t speed, uint32_t direction);

  private:
    void init_spi(int32_t SPI_SPEED);
    void init_pins();
    void init_registers_through_spi();

    static uint8_t read_register(uint8_t reg);
    static void write_register(uint8_t reg, uint8_t data);

    static void handle_error(PinInputControl* inputControl, PinOutputControl* outputControl);

    PinInputControl inputControl;
    PinOutputControl outputControl;
};