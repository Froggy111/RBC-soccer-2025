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

    // types::u8 read8(types::u8 reg);

    // void write8(types::u8 reg, types::u8 data, int8_t expected = -1);

    bool check_config();

    PinInputControl inputControl;
    PinOutputControl outputControl;
    PinSelector pinSelector;

public:

    void init(int id, types::u64 SPI_SPEED);

    types::u8 motion_burst_buffer[12] = {99}; //Stores data read from data burst 
    
    void read_motion_burst();
    types::u16 read_X_motion();
    types::u16 read_Y_motion();
    types::u16 read_squal();

    //TODO: Remove this from public and switch to private after testing
    types::u8 read8(types::u8 reg);
    void write8(types::u8 reg, types::u8 data, int8_t expected = -1);


    bool init_registers();
};