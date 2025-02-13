#pragma once
#include "pinmap.hpp"
#include <map>
#include "libs/utils/types.hpp"

class PinInputControl {
public:
  void init_digital(types::u8 pin, bool value);
  void init_analog(types::u8 pin, int value);
  void init(types::u8 pin);
  void write_digital(types::u8 pin, bool value);
  void write_analog(types::u8 pin, int value);
  bool get_last_value(types::u8 pin);

private:
  std::map<types::u8, std::pair<bool, int>> cache;
};

class PinOutputControl {
public:
  void init_digital(types::u8 pin);
  void init_analog(types::u8 pin);
  bool read_digital(types::u8 pin);
  int read_analog(types::u8 pin);
};