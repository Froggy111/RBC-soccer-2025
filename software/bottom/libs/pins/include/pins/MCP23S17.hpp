#pragma once

#include "pinmap.hpp"
#include "types.hpp"
#include <hardware/spi.h>
#include <pico/stdlib.h>

class MCP23S17 {
private:
  types::u8 id;
  types::u8 pin_state[17];
  spi_inst_t* spi_obj;

  void init_spi();
  void init_pins();

  void configure_spi();
  uint8_t read8(uint8_t device_address, uint8_t reg_address);
  void write8(uint8_t device_address, uint8_t reg_address, uint8_t data, uint8_t mask = 0xFF);

public:
  void init(types::u8 device_id, spi_inst_t* spi_obj);
  void reset();

  // if "output" is set to true, the MCP23S17 will write to the pin, otherwise it will read from the pin
  void init_gpio(uint8_t pin, bool on_A, bool is_output);
  void write_gpio(uint8_t pin, bool on_A, bool value);
  bool read_gpio(uint8_t pin, bool on_A);
};