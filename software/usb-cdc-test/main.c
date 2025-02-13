#include "class/cdc/cdc_device.h"
#include "pico/stdlib.h"
#include "tusb.h"

int main() {
  tusb_init();
  tud_task();
  const uint LED_BUILTIN = 25;
  const char text[] = "hello world!\n";
  gpio_init(LED_BUILTIN);
  gpio_set_dir(LED_BUILTIN, GPIO_OUT);

  // Wait for USB connection, calling tud_task()!
  do {
    tud_task();
    sleep_ms(1);
  } while (!tud_cdc_connected());

  while (1) {
    gpio_put(LED_BUILTIN, 1);
    for (int i = 0; i < 50; i++) {
      sleep_ms(1);
    }
    tud_task();

    // Check if there's enough space in the TX buffer before writing
    if (tud_cdc_write_available() >= sizeof(text)) {
      tud_cdc_write(text, sizeof(text) - 1); // send hello world
      tud_cdc_write_flush();                 // Flush immediately
    }

    gpio_put(LED_BUILTIN, 0);
    for (int i = 0; i < 50; i++) {
      sleep_ms(1);
    }
  }

  return 0;
}
