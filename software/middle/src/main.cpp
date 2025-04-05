extern "C" {
#include <pico/stdlib.h>
#include <hardware/clocks.h>
}
#include "comms.hpp"
#include "debug.hpp"
#include "IR.hpp"
#include "actions/LEDs.hpp"

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

  // xTaskCreate(led_blinker_task, "led_blinker_task", 1024, NULL, 10,
  //             &led_blinker_handle);

  // led_blinker_data_mutex = xSemaphoreCreateMutex();
  // bool led_attach_successful = comms::USB_CDC.attach_listener(
  //     comms::RecvIdentifiers::LEDs, led_blinker_handle, led_blinker_data_mutex,
  //     led_blinker_buffer, sizeof(led_blinker_task_data));

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(portMAX_DELAY));
  }
}

int main() {
  set_sys_clock_hz(SYS_CLK_HZ, true);
  // * Init USB Comms
  comms::init();

  xTaskCreate(main_task, "main_task", 1024, NULL, 10, NULL);

  vTaskStartScheduler();
  return 0;
}
