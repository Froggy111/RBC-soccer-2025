#include "WS2812.hpp"
#include "comms.hpp"
#include "types.hpp"

using namespace types;

const u8 LED_PIN = 25;

WS2812 led_strip(0, 6, pio0, 0, WS2812::DataByte::GREEN, WS2812::DataByte::BLUE, WS2812::DataByte::RED);

void led_shiny_shiny(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);
  while (true) {
    led_strip.fill(WS2812::RGB(0, 0, 0));
    led_strip.show();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    led_strip.fill(WS2812::RGB(255, 255, 255));
    led_strip.show();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
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
