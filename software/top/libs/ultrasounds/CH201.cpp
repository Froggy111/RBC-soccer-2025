extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
}
#include "CH201.hpp"
#include "pinmap.hpp"

Ultrasound::init() {
  // Configure I2C pins
  gpio_set_function(pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function(pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(pinmap::Pico::I2C0_SDA);
  gpio_pull_up(pinmap::Pico::I2C0_SCL);


  
}