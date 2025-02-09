#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <cstdint>
#include <pico/stdlib.h>
#include "pinmap.hpp"

// #define SPI_PORT0 spi0
// #define SPI_PORT1 spi1

enum consts{
    SPI_PORT0 = spi0,
    SPI_PORT1 = spi1
}