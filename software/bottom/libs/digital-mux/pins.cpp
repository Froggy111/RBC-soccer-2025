#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <cstdint>
#include <cassert>
#include <pico/stdlib.h>
#include "pins.hpp"
#include "pinmap.hpp"

class MCP23S17{
    int32_t MISO, MOSI, SCLK, CS;
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

    void write8(int32_t bytes, uint8_t reg_address, uint8_t data[]) {
        uint8_t com_data[bytes + 1];
        com_data[0] = reg_address & 0x7F;
        for (int byte = 1; byte < bytes; byte++) {
            com_data[byte] = data[byte];
        }
        gpio_put(CS, 0);
        spi_write_blocking((port_num ? SPI_PORT1 : SPI_PORT0), com_data, bytes);
        gpio_put(CS, 1);
    }

    uint8_t read8(uint8_t reg_address) {
        uint8_t buffer[1];
        uint8_t reg = reg_address | 0X80;
        gpio_put(CS, 0);
        spi_write_blocking((port_num ? SPI_PORT1 : SPI_PORT0), &reg, 1);
        spi_read_blocking((port_num ? SPI_PORT1 : SPI_PORT0), 0, buffer, 1);
        gpio_put(CS, 1);

        return buffer[0];
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

    void write8(int32_t pin, int32_t bytes, uint8_t reg_address, uint8_t data[]) {
        switch(pin){
            case mux1a_CS:
                mux1.write8(bytes, reg_address, data);
                break;
            case mux2a_CS:
                mux1.write8(bytes, reg_address, data);
                break;
        }
    }

    uint8_t read8(int32_t pin, uint8_t reg_address){
        uint8_t data;
        switch(pin){
            case mux1a_CS:
                mux1.read8(reg_address);
                break;
            case mux2a_CS:
                mux2.read8(reg_address);
                break;
        }
    }
};