#include "config.hpp"

TopPlateConfig get_config() {
  TopPlateConfig config;
  config.poll_interval = 1; // in ms
  config.LED_count = 16;
  return config;
}