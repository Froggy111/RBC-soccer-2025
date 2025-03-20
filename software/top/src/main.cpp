extern "C" {
#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <hardware/i2c.h>
}
#include "comms.hpp"
#include "sensors/IMUs.hpp"
#include "actions/LEDs.hpp"

#define LED_PIN 25

int main() {
  // * Init LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // * Init USB Comms
  comms::USB_CDC.init();

  // * Init SPIs
  if (!spi_init(spi0, 4000000)) {
    comms::USB_CDC.printf("SPI Initialized\n");
    return -1;
  } else {
    comms::USB_CDC.printf("SPI\n");
  }

  comms::USB_CDC.init();

  xTaskCreate(imu_poll_task, "imu_poll_task", 1024, NULL, 10, NULL);
  xTaskCreate(led_blinker_task, "led_blinker_task", 1024, NULL, 10, NULL);

  bool led_attach_successful = comms::USB_CDC.attach_listener(
      comms::RecvIdentifiers::LEDs, led_blinker_handle, led_blinker_data_mutex,
      led_blinker_buffer, sizeof(led_blinker_task_data));

  if (!led_attach_successful) {
    comms::USB_CDC.printf("LED Listener could not be attached");
    while (true) {
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
    }
  }

  vTaskStartScheduler();
}