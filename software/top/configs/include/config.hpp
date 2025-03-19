#pragma once

extern "C" {
#include <cstdint>
}

struct TopPlateConfig {
  uint8_t poll_interval;
  uint8_t LED_count;
};;


TopPlateConfig get_config();