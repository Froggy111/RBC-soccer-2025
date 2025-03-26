#pragma once
#include "pins/ADS1115.hpp"
#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include <map>

namespace driver {
class PinOutputControl {
public:
  void init(bool dbg, spi_inst_t *spi_obj);

  // * Digital Pins
  void init_digital(types::u8 pin, bool value, PinInterface interface = GPIO);
  void write_digital(types::u8 pin, bool value, PinInterface interface = GPIO);
  bool get_last_value_digital(types::u8 pin);

  // * PWM Pins
  void init_pwm(types::u8 pin, int value);
  void write_pwm(types::u8 pin, int value);

private:
  std::map<types::u8, bool> digital_cache;
  std::map<types::u8, bool> pwm_cache;

  bool debug;
  MCP23S17 dmux1;
  MCP23S17 dmux2;
};

class PinInputControl {
public:
  void init(bool dbg, spi_inst_t *spi_obj);

  // * Digital Pins
  void init_digital(types::u8 pin, PinInterface interface = GPIO);
  void pullup_digital(types::u8 pin, PinInterface interface = GPIO);
  bool read_digital(types::u8 pin, PinInterface interface = GPIO);

  // * Analog Pins
  int16_t read_analog(types::u8 pin, PinInterface interface);

private:
  bool debug;
  MCP23S17 dmux1;
  MCP23S17 dmux2;
  PICO_ADS1115 adc1;
  PICO_ADS1115 adc2;
};
}