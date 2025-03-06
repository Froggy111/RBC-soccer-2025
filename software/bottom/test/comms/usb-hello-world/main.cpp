#include "comms/usb.hpp"
#include "types.hpp"

using namespace types;

const u8 LED_PIN = 25;

void hello_world_task(void *args) {
  usb::CDC *cdc = (usb::CDC *)args;
  u8 payload[] = "Hello world!\n";
  while (true) {
    gpio_put(LED_PIN, 0);
    sleep_ms(500);
    cdc->write((comms::SendIdentifiers)payload[0], &payload[1],
               sizeof(payload) - 1);
    gpio_put(LED_PIN, 1);
    sleep_ms(500);
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);
  usb::CDC cdc = usb::CDC();
  cdc.init();
  xTaskCreate(hello_world_task, "hello_world_task", 1024, &cdc, 10, NULL);
  vTaskStartScheduler();
  return 0;
}
