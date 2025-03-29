#include "comms.hpp"
#include "pinmap.hpp"
#include "pins/types.hpp"
#include "types.hpp"
#include <hardware/gpio.h>
#include "pinmap.hpp"
#include "pins/digital_pins.hpp"

using namespace types;

const u8 LED_PIN = 25;

void kicker_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  pins::digital_pins.init();
  pins::digital_pins.set_mode(pinmap::Digital::KICK,
                              pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.write(pinmap::Digital::KICK, 0);

  while (true) {
    pins::digital_pins.write(pinmap::Digital::KICK, 0);
    pins::digital_pins.write(pinmap::Digital::KICK, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    pins::digital_pins.write(pinmap::Digital::KICK, 0);
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.init();

  xTaskCreate(kicker_task, "kicker_task", 1024, NULL, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}
