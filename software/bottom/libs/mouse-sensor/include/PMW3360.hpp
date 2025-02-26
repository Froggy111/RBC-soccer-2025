#pragma once
#include "types.hpp"
#include "dbg_pins.hpp"
#include "pin_selector.hpp"
#include <pico/types.h>
#include <string>

class MouseSensor{
private:
    void init_spi(types::u64 SPI_SPEED);

    void init_pins();

    types::u8 read8(types::u8 reg);

    bool write8(types::u8 reg, types::u8 data, int8_t expected = -1);

    bool check_config();

public:

    void init(int id, types::u64 SPI_SPEED);

    types::u8 motion_burst_buffer[12] = {0}; //Stores data read from data burst 
    
    void read_motion_burst();
    types::u16 read_X_motion();
    types::u16 read_Y_motion();
};