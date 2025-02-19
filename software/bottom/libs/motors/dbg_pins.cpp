#include "dbg_pins.hpp"
extern "C" {
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <stdio.h>
}

// pins responsible for providing input to DRV8244
void PinInputControl::init_digital(types::u8 pin, bool value) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_IN);
  write_digital(pin, value);
}

void PinInputControl::init_analog(types::u8 pin, int value) {
  // help
}

void PinInputControl::init(types::u8 pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_IN);
  cache[pin] = {0, false};
  if (cache.find(pin) == cache.end()) {
    printf("Pin not initialized!!!! pin %d\n", pin);
    return;
  }
}

void PinInputControl::write_digital(types::u8 pin, bool value) {
  if (cache.find(pin) == cache.end()) {
    printf("Pin not initialized! pin %d\n", pin);
    return;
  }
  gpio_put(pin, value);
  cache[pin] = {0, value};
}

void PinInputControl::write_analog(types::u8 pin, int value) {
  // help
  cache[pin] = {1, value};
}

bool PinInputControl::get_last_value(types::u8 pin) {
  if (this->cache.find(pin) == this->cache.end()) {
    printf("Pin not initialized! pin %d\n", pin);
    // TODO: Fix return value
    return false;
  }
  return this->cache[pin].second;
}

// pins responsible for providing output to DRV8244
void PinOutputControl::init_digital(types::u8 pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  read_digital(pin);
}

void PinOutputControl::init_analog(types::u8 pin) {
  // help
}

bool PinOutputControl::read_digital(types::u8 pin) { return gpio_get(pin); }

int PinOutputControl::read_analog(types::u8 pin) {
  // help
}