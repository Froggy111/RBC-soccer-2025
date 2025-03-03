#include "comms/usb.hpp"
#include "types.hpp"

extern "C" {
#include <pico/stdlib.h>
}

using namespace types;

const u8 LED_PIN = 25;

void hello_world_task(void *args) {
  usb::CDC *cdc = (usb::CDC *)args;
  u8 payload[] = "Hello world!\n";
  while (true) {
    cdc->write((comms::SendIdentifiers)payload[0], &payload[1],
               sizeof(payload) - 1);
    sleep_ms(1000);
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);
  usb::CDC cdc = usb::CDC();
  if (!cdc.init()) {
    sleep_ms(5000);
    while (true) {
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
    };
  }
  sleep_ms(5000);
  gpio_put(LED_PIN, 0);
  if (!cdc.wait_for_host_connection(5 * 1000 * 1000)) {
    sleep_ms(5000);
    while (true) {
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
    };
  }
  sleep_ms(5000);
  gpio_put(LED_PIN, 1);
  // if (!cdc.wait_for_CDC_connection(5 * 1000 * 1000)) {
  //   sleep_ms(5000);
  //   while (true) {
  //     gpio_put(LED_PIN, 1);
  //     sleep_ms(100);
  //     gpio_put(LED_PIN, 0);
  //     sleep_ms(100);
  //   };
  // }
  // sleep_ms(5000);
  // gpio_put(LED_PIN, 0);
  xTaskCreate(hello_world_task, "hello_world_task", 1024, &cdc, 10, NULL);
  vTaskStartScheduler();
  return 0;
}
