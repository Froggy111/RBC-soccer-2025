#include "comms.hpp"
extern "C" {
#include <pico/stdlib.h>
}
#include "comms.hpp"
#include "actions/LEDs.hpp"

int main_task() {
  // * Init LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  debug::info("USB CDC connected.\n");

  // xTaskCreate(led_blinker_task, "led_blinker_task", 1024, NULL, 10,
  //             &led_blinker_handle);

  // led_blinker_data_mutex = xSemaphoreCreateMutex();
  // bool led_attach_successful = comms::USB_CDC.attach_listener(
  //     comms::RecvIdentifiers::LEDs, led_blinker_handle, led_blinker_data_mutex,
  //     led_blinker_buffer, sizeof(led_blinker_task_data));

  // if (!led_attach_successful) {
  //   urgent_blink();
  // }

  vTaskStartScheduler();
}


int main() {
  // * Init USB Comms
  comms::init();

  xTaskCreate(main_task, "main_task", 1024, NULL, 10, &main_task_handle);

  vTaskStartScheduler();
  return 0;
}
