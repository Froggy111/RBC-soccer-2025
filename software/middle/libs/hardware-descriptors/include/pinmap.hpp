#pragma once
extern "C" {
#include "types.hpp"
}

namespace pinmap {

enum class Pico : types::u8 {
  IR9 = 0,
  IR10 = 1,
  IR11 = 2,
  IR12 = 3,
  IR13 = 4,
  IR14 = 5,
  IR15 = 6,
  IR16 = 7,
  IR17 = 8,
  IR18 = 9,
  IR19 = 10,
  IR20 = 11,
  IR21 = 12,
  IR22 = 13,
  IR23 = 14,
  IR24 = 15,
  IR8 = 34,
  IR7 = 32,
  IR6 = 21,
  IR5 = 20,
  IR4 = 19,
  IR3 = 18,
  IR1 = 22,
  IR2 = 21,
  LED_SIG_3V3 = 26,
  LED_SIG_DIR = 22,
};
}