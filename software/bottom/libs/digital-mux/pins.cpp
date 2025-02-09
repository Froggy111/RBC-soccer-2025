#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <cstdint>
#include <cassert>
#include <pico/stdlib.h>
#include "pins.hpp"
#include "pinmap.hpp"

class MCP23S17{
    public:int32_t MISO, MOSI, SCLK, CS;
    public:int32_t port_num, baud;
    public:void init(int32_t miso, int32_t mosi, int32_t sclk, int32_t cs, int32_t port,
        int32_t baudrate){
        MISO = miso, MOSI = mosi, SCLK = sclk, CS = cs, baud = baudrate;
        if (port == 0) {
            port_num = 0;
        } else if (port == 1) {
            port_num = 1;
        }
      
        
    }
};

class Pins {

    // enum pins{

    // }
    MCP23S17 mux1, mux2;
    void init(){
        
        mux1.init(mux1a_MISO, mux1a_MOSI, mux1a_SCLK, mux1a_CS, 0, 500000);
        mux2.init(mux2a_MISO, mux2a_MOSI, mux2a_SCLK, mux2a_CS, 1, 500000);
    }

    void mux_spi_init(int32_t port){

        assert(port == 0 or port == 1);

        if(port == 0){
            spi_init(SPI_PORT0, mux1.baud);
            gpio_set_function(mux1a_MISO, GPIO_FUNC_SPI);
            gpio_set_function(mux1a_SCLK, GPIO_FUNC_SPI);
            gpio_set_function(mux1a_MOSI, GPIO_FUNC_SPI);
        
            gpio_init(mux1a_CS);
            gpio_set_dir(mux1a_CS, GPIO_OUT);
            gpio_put(mux1a_CS, 1);
        } else {
            spi_init(SPI_PORT1, mux2.baud);
            gpio_set_function(mux2a_MISO, GPIO_FUNC_SPI);
            gpio_set_function(mux2a_SCLK, GPIO_FUNC_SPI);
            gpio_set_function(mux2a_MOSI, GPIO_FUNC_SPI);
        
            gpio_init(mux2a_CS);
            gpio_set_dir(mux2a_CS, GPIO_OUT);
            gpio_put(mux2a_CS, 1);
        }


    }
};