#include "config.hpp"
#include "types.hpp"
#include "comms.hpp"
#include "WS2812.hpp"
#include "pinmap.hpp"

using namespace types;


enum LED_MODE { RED = 1 };

struct LEDBlinkerData {
  LED_MODE mode = LED_MODE::RED;
  void reset(void) { mode = LED_MODE::RED; }
};

WS2812 led_strip((uint)pinmap::Pico::LED_SIG_3V3, get_config().LED_count, pio0, 0,
                 WS2812::DataFormat::FORMAT_GRB);

TaskHandle_t led_blinker_handle = nullptr;
LEDBlinkerData led_blinker_task_data = {};
u8 led_blinker_buffer[sizeof(led_blinker_task_data)] = {0};
SemaphoreHandle_t led_blinker_data_mutex = nullptr;

void led_blinker_task(void *args) {
  for (;;) {
    TickType_t previous_wait_time = xTaskGetTickCount();

    led_blinker_task_data.reset();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(led_blinker_data_mutex, portMAX_DELAY);
    memcpy(&led_blinker_task_data, led_blinker_buffer, sizeof(LEDBlinkerData));
    memset(led_blinker_buffer, 0, sizeof(led_blinker_buffer));
    xSemaphoreGive(led_blinker_buffer);

    switch (led_blinker_task_data.mode) {
    case (LED_MODE::RED):
      led_strip.fill(WS2812::RGB(255, 0, 0));
      led_strip.show();
      break;
    default:
      break;
    }
    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(get_config().poll_interval));
  }
}