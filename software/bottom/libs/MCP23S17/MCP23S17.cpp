#include "MCP23S17.hpp"
#include "pico.h"

#define SCLK    4
#define MOSI    5
#define MISO    6
#define CS0     7
#define CS1     9


MUX mux0 = *new MUX(MISO, MOSI, SCLK, CS0, 0, 500000);
