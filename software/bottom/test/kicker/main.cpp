#include "comms.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include <hardware/gpio.h>
#include "pinmap.hpp"

using namespace types;

const u8 LED_PIN = 25;

void kicker_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  gpio_init((uint)pinmap::Pico::KICK);
  gpio_set_dir((uint)pinmap::Pico::KICK, GPIO_OUT);

  while (true) {
    gpio_put((uint)pinmap::Pico::KICK, 0);
    gpio_put((uint)pinmap::Pico::KICK, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_put((uint)pinmap::Pico::KICK, 0);
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);+
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.init();

  xTaskCreate(kicker_task, "kicker_task", 1024, NULL, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}
