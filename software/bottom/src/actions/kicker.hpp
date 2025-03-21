#pragma once

#include "comms/usb.hpp"
#include "projdefs.h"
#include "types.hpp"
#include "pinmap.hpp"
#include "hardware/gpio.h"
#include "comms.hpp"

using namespace types;

struct KickerData {
  uint16_t pulse_duration = 100;
};

TaskHandle_t kicker_task_handle = nullptr;
KickerData kicker_task_data = {};
u8 kicker_task_buffer[sizeof(kicker_task_data)];
SemaphoreHandle_t kicker_mutex = nullptr;

void kicker_task(void *args) {
  gpio_init((uint)pinmap::Pico::KICK);
  gpio_set_dir((uint)pinmap::Pico::KICK, GPIO_OUT);

  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(kicker_mutex, portMAX_DELAY);
    memcpy(&kicker_task_data, kicker_task_buffer,
           sizeof(kicker_task_data));
    memset(kicker_task_buffer, 0, sizeof(kicker_task_buffer));
    xSemaphoreGive(kicker_mutex);

    gpio_put((uint)pinmap::Pico::KICK, 0);
    gpio_put((uint)pinmap::Pico::KICK, 1);
    vTaskDelay(pdMS_TO_TICKS(kicker_task_data.pulse_duration));
    gpio_put((uint)pinmap::Pico::KICK, 0);

    vTaskDelay(pdMS_TO_TICKS(KICK_MINIMUM_INTERVAL)); // kick only every 5 seconds min
  }
}