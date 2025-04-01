#include "WS2812.hpp"
#include "comms.hpp"
#include "types.hpp"
#include "pinmap.hpp"
#include <hardware/gpio.h>

using namespace types;

const u8 LED_PIN = 25;

WS2812 led_strip((uint)pinmap::Pico::LED_SIG_3V3, 24, pio0, 0,
                 WS2812::DataFormat::FORMAT_GRB);

void led_shiny_shiny(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  // initialize the direction pin
  gpio_init((uint)pinmap::Pico::LED_SIG_DIR);
  gpio_set_dir((uint)pinmap::Pico::LED_SIG_DIR, GPIO_OUT);
  gpio_put((uint)pinmap::Pico::LED_SIG_DIR, 1);

  uint8_t offset = 0;

  while (true) {
    // Rainbow wheel animation
    for (int i = 0; i < 24; i++) {
      // Calculate hue based on position and offset
      uint8_t hue = ((i * 256 / 24) + offset) % 256;
      led_strip.setPixelColor(i, WS2812::hue_to_rgb(hue));
    }

    led_strip.show();
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Increment offset to make the wheel spin
    offset = (offset + 5) % 256;
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::init();

  xTaskCreate(led_shiny_shiny, "led_shiny_shiny", 1024, NULL, 10, NULL);

  vTaskStartScheduler();
  return 0;
}