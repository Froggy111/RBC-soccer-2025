extern "C" {
#include <pico/stdlib.h>
}
#include "comms.hpp"
#include "debug.hpp"
#include "IR.hpp"

const int LED_PIN = 25;

TaskHandle_t main_task_handle = nullptr;
void main_task(void *args) {
  // * Init LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  debug::info("USB CDC connected.\n");
  IR::init();

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(portMAX_DELAY));
  }
}

int main() {
  // * Init USB Comms
  comms::init();

  xTaskCreate(main_task, "main_task", 1024, NULL, 10, NULL);

  vTaskStartScheduler();
  return 0;
}
