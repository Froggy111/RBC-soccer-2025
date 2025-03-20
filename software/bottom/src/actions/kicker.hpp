#pragma once

#include "types.hpp"
#include "pinmap.hpp"
#include "hardware/gpio.h"
#include "comms.hpp"

using namespace types;

TaskHandle_t kicker_task_handle = nullptr;

void kicker_task(void *args) {
  gpio_init((uint)pinmap::Pico::KICK);
  gpio_set_dir((uint)pinmap::Pico::KICK, GPIO_OUT);

  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }
}