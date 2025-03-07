#include "comms.hpp"
#include "types.hpp"
#include "ALSPT19.hpp"

using namespace types;

const u8 LED_PIN = 25;
LineSensor line_sensor = LineSensor();

void line_sensor_poll_task(void *args) {
  usb::CDC *cdc = (usb::CDC *)args;

  while (true) {
    for (int i = 0; i < 48; i++) {
      uint16_t val = line_sensor.read_raw(i);
      cdc->printf("Line sensor %d: %d\n", i, val);
    }
    sleep_ms(1000);
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  usb::CDC cdc = usb::CDC();

  cdc.init();
  line_sensor.init(1, NULL);

  xTaskCreate(line_sensor_poll_task, "line_sensor_poll_task", 1024, &cdc, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}
