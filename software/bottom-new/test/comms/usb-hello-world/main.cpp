#include "comms.hpp"
#include "types.hpp"
#include "debug.hpp"

using namespace types;

const u8 LED_PIN = 25;

void hello_world_task(void *args) {
  char payload[] = "Hello world!\n";
  while (true) {
    gpio_put(LED_PIN, 0);
    sleep_ms(500);
    debug::log(payload);
    gpio_put(LED_PIN, 1);
    sleep_ms(500);
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);
  comms::init();
  xTaskCreate(hello_world_task, "hello_world_task", 1024, NULL, 10, NULL);

  vTaskStartScheduler();
  return 0;
}
