#pragma once
#include <map>
#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include "pin_selector.hpp"

namespace driver {
class PinInputControl {
public:
  void init(bool dbg, spi_inst_t* spi_obj);
  void init_digital(types::u8 pin, bool value, PinInterface interface = GPIO);
  void init_analog(types::u8 pin, int value);
  void write_digital(types::u8 pin, bool value, PinInterface interface = GPIO);
  void write_analog(types::u8 pin, int value);
  bool get_last_value_digital(types::u8 pin);
  bool get_last_value_analog(types::u8 pin);

private:
  std::map<types::u8, bool> digital_cache;
  std::map<types::u8, bool> analog_cache;
  bool debug;
  MCP23S17 dmux1;
  MCP23S17 dmux2;
};

class PinOutputControl {
public:
  void init(bool dbg, spi_inst_t* spi_obj);
  void init_digital(types::u8 pin, PinInterface interface = GPIO);
  void init_analog(types::u8 pin);
  bool read_digital(types::u8 pin, PinInterface interface = GPIO);
  int read_analog(types::u8 pin);

private:
  bool debug;
  MCP23S17 dmux1;
  MCP23S17 dmux2;
};
}