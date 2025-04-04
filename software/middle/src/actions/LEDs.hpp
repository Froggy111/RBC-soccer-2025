#pragma once

#include "config.hpp"
#include "projdefs.h"
#include "types.hpp"
#include "comms.hpp"
#include "WS2812.hpp"
#include "pinmap.hpp"
#include <cstdint>
#include <pico/types.h>

using namespace types;

struct LEDData {
  uint8_t RED = 0;
  uint8_t GREEN = 0;
  uint8_t BLUE = 0;
};

struct LEDBLinkerData {
  uint8_t id = 0;
  uint8_t RED = 0;
  uint8_t GREEN = 0;
  uint8_t BLUE = 0;
} __attribute__((packed));

WS2812 led_strip((uint)pinmap::Pico::LED_SIG_3V3, LED_COUNT, pio0,
                 0, WS2812::DataFormat::FORMAT_GRB);

TaskHandle_t led_blinker_handle = nullptr;
LEDData led_states[LED_COUNT];
LEDBLinkerData led_blinker_task_data = {};
u8 led_blinker_buffer[sizeof(led_blinker_task_data)];
SemaphoreHandle_t led_blinker_data_mutex = nullptr;

void led_blinker_task(void *args) {
  // initialize the direction pin
  gpio_init((uint)pinmap::Pico::LED_SIG_DIR);
  gpio_set_dir((uint)pinmap::Pico::LED_SIG_DIR, GPIO_OUT);
  gpio_put((uint)pinmap::Pico::LED_SIG_DIR, 1);

  // reset the data
  memset(led_states, 0, sizeof(led_states));
  // make it initially green
  for (int i = 0; i < LED_COUNT; i++) {
    led_states[i].RED = 0;
    led_states[i].GREEN = 100;
    led_states[i].BLUE = 0;
  }

  for (;;) {
    for (int i = 0; i < LED_COUNT; i++) {
      led_strip.setPixelColor(i, WS2812::RGB(led_states[i].RED,
                                             led_states[i].GREEN,
                                             led_states[i].BLUE));
      vTaskDelay(pdMS_TO_TICKS(1));
    }
    led_strip.show();

    // * data transfer
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(led_blinker_data_mutex, portMAX_DELAY);
    memcpy(&led_blinker_task_data, led_blinker_buffer,
           sizeof(led_blinker_task_data));
    memset(led_blinker_buffer, 0, sizeof(led_blinker_buffer));
    xSemaphoreGive(led_blinker_data_mutex);

    // * based on the id, set the color

    if (led_blinker_task_data.id >= 0 && led_blinker_task_data.id < LED_COUNT) {
      led_states[led_blinker_task_data.id].RED =
          led_blinker_task_data.RED;
      led_states[led_blinker_task_data.id].GREEN =
          led_blinker_task_data.GREEN;
      led_states[led_blinker_task_data.id].BLUE =
          led_blinker_task_data.BLUE;
    }
  }
}