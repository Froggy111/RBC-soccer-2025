#include "libs/hardware-descriptors/pinmap.hpp"
#include "pico/stdlib.h"
#include <iostream>
#include <map>

// pins responsible for providing input to MCP23S17
class PinInputControl {
  std::map<pinmap::Pins, bool> cache;

  void init(pinmap::Pins pin, bool value) {
    int32_t pinnum;
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    write(pin, value);
  }

  void init(pinmap::Pins pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
  }

  bool write(pinmap::Pins pin, bool value) {
    if (this->cache.find(pin) == this->cache.end()) {
      throw std::runtime_error("Pin not initialized");
    }
    gpio_put(pin, value);
    this->cache[pin] = value;
  }

  bool get_last_value(pinmap::Pins pin) {
    if (this->cache.find(pin) == this->cache.end()) {
      throw std::runtime_error("Pin not initialized");
    }
    return this->cache[pin];
  }
};

// pins responsible for providing output from MCP23S17
class PinOutputControl {
  void init_with_value(pinmap::Pins pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    this->read(pin);
  }

  bool read(pinmap::Pins pin) { return gpio_get(pin); }
};
