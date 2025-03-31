#pragma once
#include <map>
#include "pinmap.hpp"
#include "types.hpp"

namespace mouse {
class PinInputControl {
public:
  void init_digital(types::u8 pin, bool value);
  void init_analog(types::u8 pin, int value);
  void write_digital(types::u8 pin, bool value);
  void write_analog(types::u8 pin, int value);
  bool get_last_value_digital(types::u8 pin);
  bool get_last_value_analog(types::u8 pin);

private:
  std::map<types::u8, bool> digital_cache;
  std::map<types::u8, bool> analog_cache;
};

class PinOutputControl {
public:
  void init_digital(types::u8 pin);
  void init_analog(types::u8 pin);
  bool read_digital(types::u8 pin);
  int read_analog(types::u8 pin);
};
}