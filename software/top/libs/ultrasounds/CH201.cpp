extern "C" {
#include <pico/stdlib.h>
#include <invn/soniclib/soniclib.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
}
#include "CH201.hpp"
#include "pinmap.hpp"
#include "pins/MCP23S17.hpp"

void Ultrasound::group_init() {
  dmux1 = new MCP23S17();
  dmux1->init(1, 0);
  dmux2 = new MCP23S17();
  dmux2->init(2, 0);
}

void Ultrasound::init(i2c_inst_t * i2c_inst, int us_id) {
  i2c = i2c_inst;
  id = us_id;

  // Configure I2C pins
  gpio_set_function((uint) pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint) pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint) pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint) pinmap::Pico::I2C0_SCL);

  // Configure GPIO pins
  gpio_init((uint) pinmap::Pico::US_NRST);
  gpio_set_dir((uint)pinmap::Pico::US_NRST, GPIO_OUT);

  // init prog and int
  set_prog(0, true);
  set_int(0, true);
}

void Ultrasound::set_prog(uint8_t prog, bool init) {
  if (id >= 1 && id <= 8) {
    if (init)
      dmux1->init_gpio((id - 1) % 4 * 2 + 1, id >= 1 && id <= 4, true);
    dmux1->write_gpio((id - 1) % 4 * 2 + 1, id >= 1 && id <= 4, prog);
  } else if (id >= 9 && id <= 16) {
    if (init)
      dmux2->init_gpio((id - 1) % 4 * 2 + 1, id >= 9 && id <= 12, true);
    dmux2->write_gpio((id - 1) % 4 * 2 + 1, id >= 9 && id <= 12, prog);
  }
}

void Ultrasound::set_int(uint8_t int_val, bool init) {
  if (id >= 1 && id <= 8) {
    if (init)
      dmux1->init_gpio((id - 1) % 4 * 2, id >= 1 && id <= 4, true);
    dmux1->write_gpio((id - 1) % 4 * 2, id >= 1 && id <= 4, int_val);
  } else if (id >= 9 && id <= 16) {
    if (init)
      dmux2->init_gpio((id - 1) % 4 * 2, id >= 9 && id <= 12, true);
    dmux2->write_gpio((id - 1) % 4 * 2, id >= 9 && id <= 12, int_val);
  }
}

void Ultrasound::reset() {
  gpio_put((uint) pinmap::Pico::US_NRST, 0);
  sleep_ms(1);
  gpio_put((uint) pinmap::Pico::US_NRST, 1);
  sleep_ms(1);
}

// * i2c functions
static uint8_t i2c_read_reg(ch_dev_t *dev_ptr, uint8_t reg_addr, uint8_t *data_ptr, uint16_t num_bytes) {
  i2c_inst_t* i2c = (i2c_inst_t*)dev_ptr->bus_index;
  uint8_t sensor_addr = dev_ptr->i2c_address;
  
  i2c_write_blocking(i2c, sensor_addr, &reg_addr, 1, true);  // true to keep master control of bus
  int bytes_read = i2c_read_blocking(i2c, sensor_addr, data_ptr, num_bytes, false);
  
  return (bytes_read == num_bytes) ? 0 : 1;
}

static uint8_t i2c_write_reg(ch_dev_t *dev_ptr, uint8_t reg_addr, uint8_t *data_ptr, uint16_t num_bytes) {
  i2c_inst_t* i2c = (i2c_inst_t*)dev_ptr->bus_index;
  uint8_t sensor_addr = dev_ptr->i2c_address;
  
  uint8_t buf[num_bytes + 1];
  buf[0] = reg_addr;
  memcpy(buf + 1, data_ptr, num_bytes);
  
  int bytes_written = i2c_write_blocking(i2c, sensor_addr, buf, num_bytes + 1, false);
  
  return (bytes_written == (num_bytes + 1)) ? 0 : 1;
}

// * get distance

uint32_t Ultrasound::get_dist(ch_range_t req_range) {
  uint32_t range_mm = 0;
  ch_get_range(&ch201_sensor, req_range, &range_mm);
  return range_mm;
}
