#include "comms.hpp"
#include "types.hpp"
#include "CH201.hpp"  

using namespace types;

const u8 LED_PIN = 25;

// remember to init!

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  usb::CDC cdc = usb::CDC();

  cdc.init();

  xTaskCreate(ultrasounds_poll_task, "ultrasounds_poll_task", 1024, &cdc, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}
