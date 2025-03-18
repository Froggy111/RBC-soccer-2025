#include "comms.hpp"
#include "types.hpp"
#include "TSSP4038.hpp"
#include "hardware/gpio.h"

using namespace types;

const u8 LED_PIN = 25;
IRSensor ir_sensor = IRSensor();

void ir_sensor_poll_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  while (true) {
    ir_sensor.read_raw();

    for (int i = 0; i < 24; i++) {
      comms::USB_CDC.printf("IR sensor %d: %d\r\n", i+1, avg_values[i]);
    }
    sleep_ms(1000);
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.init();

  xTaskCreate(ir_sensor_poll_task(void *args), "ir_sensor_poll_task", 1024, NULL, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}
