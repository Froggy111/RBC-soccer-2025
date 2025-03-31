#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include "IR.hpp"

using namespace types;

const u8 LED_PIN = 25;

void main_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(portMAX_DELAY);
  debug::log("Initialising IR sensors...\r\n");
  IR::init();
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::init();
  xTaskCreate(main_task, "ir_sensor_poll_task", 1024, NULL, 10, NULL);

  vTaskStartScheduler();
  return 0;
}
