extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
}
#include "CH201.hpp"
#include "pinmap.hpp"
#include "pins/MCP23S17.hpp"

Ultrasound::group_init() {
  dmux1.init(0, 0);
  dmux2.init(1, 0);
}

Ultrasound::init(i2c_inst_t * i2c_inst, int device_id) {
  i2c = i2c_inst;
  id = device_id;

  // Configure I2C pins
  gpio_set_function(pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function(pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(pinmap::Pico::I2C0_SDA);
  gpio_pull_up(pinmap::Pico::I2C0_SCL);

  // Configure GPIO pins
  gpio_init(pinmap::Pico::US_NRST);
  gpio_set_dir(pinmap::Pico::US_NRST, GPIO_OUT);

  // * Load firmware to CH201
  
}

Ultrasound::reset() {
  gpio_put(pinmap::Pico::US_NRST, 0);
  sleep_ms(1);
  gpio_put(pinmap::Pico::US_NRST, 1);
  sleep_ms(1);
}
