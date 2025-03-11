#include "comms.hpp"
extern "C" {
#include <pico/stdlib.h>

#define LEN_THRESH 8 // whacky hack
#include <invn/soniclib/soniclib.h>
#include <invn/soniclib/chirp_bsp.h>
#include <invn/soniclib/sensor_fw/ch201/ch201_gprmt.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
}
#include "CH201.hpp"
#include "pinmap.hpp"
#include "pins/MCP23S17.hpp"

#define CH201_RTC_PULSE_SEQUENCE_TIME_LENGTH 1 // in ms
#define CH201_COUNT 16

// TODO: Possibly calibrate the RTC clock

void Ultrasound::group_init() {
  dmux1 = new MCP23S17();
  dmux1->init(1, 0);
  dmux2 = new MCP23S17();
  dmux2->init(2, 0);

  // init CH201 group
  if (ch_group_init(&ch201_group, 16, 1,
                    CH201_RTC_PULSE_SEQUENCE_TIME_LENGTH)) {
    comms::USB_CDC.printf("CH201 group init failed\n");
  }
}

void Ultrasound::group_start() { ch_group_start(&ch201_group); }

void Ultrasound::init(i2c_inst_t *i2c_inst, int us_id) {
  i2c = i2c_inst;
  id = us_id;

  //* PIN CONFIGURATION
  // Configure I2C pins
  gpio_set_function((uint)pinmap::Pico::I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function((uint)pinmap::Pico::I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SDA);
  gpio_pull_up((uint)pinmap::Pico::I2C0_SCL);

  // Configure GPIO pins
  gpio_init((uint)pinmap::Pico::US_NRST);
  gpio_set_dir((uint)pinmap::Pico::US_NRST, GPIO_OUT);

  // init prog and int
  set_prog(0, true);
  set_int(0, true);

  //* ULTRASOUND INITIALIZATION
  // Reset the sensor before initializing
  reset();

  ch_init(&ch201_sensor, &ch201_group, CH201_COUNT, ch201_gprmt_init);
  ch_set_init_firmware(&ch201_sensor, ch201_gprmt_init);
  ch_group_start(&ch201_group);
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
  gpio_put((uint)pinmap::Pico::US_NRST, 0);
  sleep_ms(1);
  gpio_put((uint)pinmap::Pico::US_NRST, 1);
  sleep_ms(1);
}