#include "comms/usb.hpp"
#include "types.hpp"

using namespace types;

const u8 LED_PIN = 25;

struct BlinkOnCmdData {
  u16 blink_time_ms = 0;
  void reset(void) { blink_time_ms = 0; }
};
TaskHandle_t blink_on_cmd_handle = nullptr;
BlinkOnCmdData blink_on_cmd_data = {};
u8 blink_on_cmd_data_buffer[sizeof(BlinkOnCmdData)] = {0};
SemaphoreHandle_t blink_on_cmd_data_mutex = nullptr;

void blink_on_cmd(void *args) {
  usb::CDC cdc = *(usb::CDC *)args;
  for (;;) {
    blink_on_cmd_data.reset();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(blink_on_cmd_data_mutex, portMAX_DELAY);
    memcpy(&blink_on_cmd_data, blink_on_cmd_data_buffer,
           sizeof(BlinkOnCmdData));
    memset(blink_on_cmd_data_buffer, 0, sizeof(blink_on_cmd_data_buffer));
    xSemaphoreGive(blink_on_cmd_data_mutex);
    gpio_put(LED_PIN, 1);
    // vTaskDelay(pdMS_TO_TICKS(blink_on_cmd_data.blink_time_ms));
    sleep_ms(blink_on_cmd_data.blink_time_ms);
    gpio_put(LED_PIN, 0);
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 0);
  usb::CDC cdc = usb::CDC();
  cdc.init();
  blink_on_cmd_data_mutex = xSemaphoreCreateMutex();
  xTaskCreate(blink_on_cmd, "blink_on_cmd", 1024, &cdc, 10,
              &blink_on_cmd_handle);
  if (!blink_on_cmd_handle) {
    while (true) {
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
    }
  }
  bool attached =
      cdc.attach_listener(comms::RecvIdentifiers::DEBUG_TEST_BLINK,
                          blink_on_cmd_handle, blink_on_cmd_data_mutex,
                          blink_on_cmd_data_buffer, sizeof(blink_on_cmd_data));
  if (!attached) {
    while (true) {
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
    }
  }
  vTaskStartScheduler();
  return 0;
}
