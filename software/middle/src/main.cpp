#include "comms.hpp"
extern "C" {
#include <pico/stdlib.h>
}
#include "comms.hpp"
#include "actions/LEDs.hpp"

#define LED_PIN 25

int urgent_blink() {
  while (true) {
    gpio_put(LED_PIN, 1);
    sleep_ms(100);
    gpio_put(LED_PIN, 0);
    sleep_ms(100);
  }
}

int main() {
  // * Init LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // * Init USB Comms
  comms::init();

  xTaskCreate(led_blinker_task, "led_blinker_task", 1024, NULL, 10,
              &led_blinker_handle);

  led_blinker_data_mutex = xSemaphoreCreateMutex();
  bool led_attach_successful = comms::USB_CDC.attach_listener(
      comms::RecvIdentifiers::LEDs, led_blinker_handle, led_blinker_data_mutex,
      led_blinker_buffer, sizeof(led_blinker_task_data));

  if (!led_attach_successful) {
    urgent_blink();
  }

  vTaskStartScheduler();
}
