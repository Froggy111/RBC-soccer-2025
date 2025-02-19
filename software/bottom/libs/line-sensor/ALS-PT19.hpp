#pragma once
#include "hardware/i2c.h"
#define LINE_SENSOR_HPP
#define LINE_SENSOR_ADDR 0x39 // I2C address of ALS-PT19

class LineSensor {
    public:
        LineSensor();
        bool begin();
        void read();
        void set_integration_time(uint16_t time);
        void set_gain(uint8_t gain);
    private:
        uint16_t get_lux();
        uint16_t get_raw();
        uint16_t get_integration_time();
        uint8_t get_gain();

        void write_register(uint8_t reg, uint8_t val);
        uint8_t read_register(uint8_t reg);
};






#endif
















