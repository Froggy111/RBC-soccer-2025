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
    mux1a_SCLK = 11,
    mux1a_MOSI = 12,
    mux1a_MISO = 13,
    mux1a_CS = 14
};

enum mux1b{
    mux1b_SCLK = 11,
    mux1b_MOSI = 12,
    mux1b_MISO = 13,
    mux1b_CS = 14
};

enum mux2a{
    mux2a_SCLK = 11,
    mux2a_MOSI = 12,
    mux2a_MISO = 13,
    mux2a_CS = 14
};

enum mux2b{
    mux2b_SCLK = 11,
    mux2b_MOSI = 12,
    mux2b_MISO = 13,
    mux2b_CS = 14
};