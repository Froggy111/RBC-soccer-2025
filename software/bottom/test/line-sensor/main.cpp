#include "comms.hpp"
#include "comms/usb.hpp"
#include "projdefs.h"
#include "types.hpp"
#include "ALSPT19.hpp"

// TODO: WEIRD SPI ERRORS

using namespace types;

const u8 LED_PIN = 25;
LineSensor line_sensor = LineSensor();

void line_sensor_poll_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  // for mux
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialization Failed!\r\n");
  } else {
    comms::USB_CDC.printf("SPI Initialization Successful!\r\n");
  }
  line_sensor.init(spi0);

  while (true) {
    for (int i = 0; i < 48; i++) {
      uint16_t val = line_sensor.read_raw(i);
      if (val > 1000) {
        comms::USB_CDC.printf("Line sensor %d: %d\r\n", i, val);
      }
      busy_wait_us(1);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::init();

  xTaskCreate(line_sensor_poll_task, "line_sensor_poll_task", 1024, NULL, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}
