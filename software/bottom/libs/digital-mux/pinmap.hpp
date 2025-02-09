#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <cstdint>
#include <pico/stdlib.h>

enum pinmap{
    pico_SCLK = 4,
    pico_MOSI = 5, 
    pico_MISO = 6,
    pico_CS = 7
};

enum mux1a{
    mux1a_SCLK,
    mux1a_MOSI,
    mux1a_MISO,
    mux1a_CS = 14
};

enum mux1b{
    mux1b_SCLK,
    mux1b_MOSI,
    mux1b_MISO,
    mux1b_CS = 14
};

enum mux2a{
    mux2a_SCLK,
    mux2a_MOSI,
    mux2a_MISO,
    mux2a_CS = 20
};

enum mux2b{
    mux2b_SCLK,
    mux2b_MOSI,
    mux2b_MISO,
    mux2b_CS = 14
};