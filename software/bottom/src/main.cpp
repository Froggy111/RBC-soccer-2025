#include "comms/usb.hpp"

extern "C" {
  #include <pico/stdlib.h>
  #include <hardware/spi.h>
}
#include "comms.hpp"

const int LED_PIN = 25;

int main() {
  // * Init LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // * Init USB Comms
  comms::USB_CDC.init();

  // * Init SPIs
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialized\n");
    return -1;
  } else {
    comms::USB_CDC.printf("SPI\n");
  }

  vTaskStartScheduler();
}