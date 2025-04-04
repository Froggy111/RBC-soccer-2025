#pragma once
#include "types.hpp"

namespace pinmap {

enum class RPI : types::u8 {
    PI_I2C_SDA             = 2,
    PI_I2C_SCL             = 3,
    TOPPICO_TOGGLE_RUN     = 4,
    TOPPICO_TOGGLE_EN      = 17,
    BOTTOMPICO_TOGGLE_EN   = 27,
    BOTTOMPICO_TOGGLE_RUN   = 22,
    RPI_SPI0_MOSI          = 10,
    RPI_SPI0_MISO          = 9,
    RPI_SPI0_SCLK          = 11,
    RPI_SW3                = 0,
    RPI_SW2                = 5,
    RPI_SW1                = 6,
    RPI_SW0                = 13,
    PCM_FS                 = 19,
    PI_SW5                 = 26,
    PI_UART_TXD            = 14,
    PI_UART_RXD            = 15,
    PCM_CLK                = 18,
    LRCLOCK                = 23,
    ADC_INT                = 24,
    MIDPICO_TOGGLE_EN      = 25,
    RPI_SPI0_SCS0          = 8,
    RPI_SPI0_SCS1          = 7,
    MIDPICO_TOGGLE_RUN     = 1,
    PWM0                   = 12,
    PCM_DIN = 20,
    PCM_DOUT = 21,
};

} 