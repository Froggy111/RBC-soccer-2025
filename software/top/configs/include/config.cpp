#include "types.hpp"

struct TopPlateConfig {
  types::u8 poll_interval = 1; // in ms
  types::u8 LED_count = 16;
} config;