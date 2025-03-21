extern "C" {
#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <hardware/i2c.h>
}
#include "comms.hpp"
#include "sensors/lightgate.hpp"
#include "sensors/mouse_sensor.hpp"
#include "sensors/line_sensor.hpp"
#include "actions/kicker.hpp"
#include "actions/motors.hpp"

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
  comms::USB_CDC.init();

  // * Init SPIs
  if (!spi_init(spi0, 1000000)) {
    urgent_blink();
  }

  

  vTaskStartScheduler();
}