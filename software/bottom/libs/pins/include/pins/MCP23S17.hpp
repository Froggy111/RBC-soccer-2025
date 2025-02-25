#pragma once

#include "pinmap.hpp"
#include "types.hpp"
#include <pico/stdlib.h>

class MCP23S17 {
private:
  types::u8 id;

  void init_spi(types::u32 baudrate);
  void init_pins();

  uint8_t read8(uint8_t reg_address);
  void write8(uint8_t reg_address, uint8_t data);

public:
  void init(types::u8 device_id, types::u32 baudrate);
  void reset();

  void init_gpio(uint8_t pin, bool is_output);
  void set_gpio(uint8_t pin, bool value);
  bool get_gpio(uint8_t pin);
};