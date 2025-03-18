#include "comms.hpp"
#include "types.hpp"

using namespace types;

const u8 LED_PIN = 25;
LineSensor line_sensor = LineSensor();

void led_shiny_shiny(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.init();

  xTaskCreate(led_shiny_shiny, "led_shiny_shiny", 1024, NULL, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}
