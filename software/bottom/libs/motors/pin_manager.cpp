#include "pin_manager.hpp"
#include "pins/MCP23S17.hpp"
extern "C" {
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <stdio.h>
#include <hardware/pwm.h>
}

void PinInputControl::init(bool dbg, spi_inst_t *spi_obj) {
  debug = dbg;
  dmux1.init(1, spi_obj);
  dmux2.init(2, spi_obj);
}

// pins responsible for providing input to DRV8244
void PinInputControl::init_digital(types::u8 pin, bool value, bool on_device_1, bool on_A) {
  if (debug) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
  } else {
    if (on_device_1)
      dmux1.init_gpio(pin, on_A, true);
    else
      dmux2.init_gpio(pin, on_A, true);
  }

  this->digital_cache[pin] = value;
  write_digital(pin, value);
}

void PinInputControl::init_analog(types::u8 pin, int value) {
  gpio_set_function(pin, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(pin);
  uint channel = pwm_gpio_to_channel(pin);

  // Set PWM configuration
  pwm_set_clkdiv(slice_num, 4.0);
  pwm_set_wrap(slice_num, 12500); // set max value

  // Set PWM level
  this->analog_cache[pin] = value;
  write_analog(pin, value);

  // enable PWM
  pwm_set_enabled(slice_num, true);
}

void PinInputControl::write_digital(types::u8 pin, bool value, bool on_device_1, bool on_A) {
  if (this->digital_cache.find(pin) == this->digital_cache.end()) {
    printf("Pin not initialized! pin %d\n", pin);
    return;
  }

  if (debug) {
    gpio_put(pin, value);
  } else {
    if (on_device_1) dmux1.write_gpio(pin, on_A, value);
    else dmux2.write_gpio(pin, on_A, value);
  }

  // printf("%d has been written to pin %d\n", value, pin);
  this->digital_cache[pin] = value;
}

void PinInputControl::write_analog(types::u8 pin, int value) {
  if (this->analog_cache.find(pin) == this->analog_cache.end()) {
    printf("Pin not initialized! pin %d\n", pin);
    return;
  }
  uint slice_num = pwm_gpio_to_slice_num(pin);
  uint channel = pwm_gpio_to_channel(pin);
  printf("%d has been written to pin %d\n", value, pin);
  pwm_set_chan_level(slice_num, channel, value);
}

bool PinInputControl::get_last_value_digital(types::u8 pin) {
  if (this->digital_cache.find(pin) == this->digital_cache.end()) {
    printf("Pin not initialized! pin %d\n", pin);
    // TODO: Fix return value
    return false;
  }

  return this->digital_cache[pin];
}

bool PinInputControl::get_last_value_analog(types::u8 pin) {
  if (this->analog_cache.find(pin) == this->analog_cache.end()) {
    printf("Pin not initialized! pin %d\n", pin);
    // TODO: Fix return value
    return false;
  }

  return this->analog_cache[pin];
}

void PinOutputControl::init(bool dbg, spi_inst_t *spi_obj) {
  debug = dbg;
  dmux1.init(1, spi_obj);
  dmux2.init(2, spi_obj);
}

// pins responsible for providing output to DRV8244
void PinOutputControl::init_digital(types::u8 pin, bool on_device_1, bool on_A) {
  if (debug) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
  } else {
    if (on_device_1)
      dmux1.init_gpio(pin, on_A, false);
    else
      dmux2.init_gpio(pin, on_A, false);
  }
}

bool PinOutputControl::read_digital(types::u8 pin, bool on_device_1, bool on_A) {
  if (debug) {
    bool result = gpio_get(pin);
    // printf("%d has been read from pin %d\n", result, pin);
    return result;
  } else {
    if (on_device_1)
      return dmux1.read_gpio(pin, on_A);
    else
      return dmux2.read_gpio(pin, on_A);
  }
}