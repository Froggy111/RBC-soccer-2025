#include "comms.hpp"
#include "types.hpp"
#include "CH201.hpp"
#include <hardware/i2c.h>
#include <hardware/spi.h>

using namespace types;

const u8 LED_PIN = 25;

Ultrasound *ultrasound_group[20];

void ultrasounds_poll_task(void *args) {
  usb::CDC *cdc = (usb::CDC *)args;
  cdc->wait_for_CDC_connection(0xFFFFFFFF);

  // Initialize SPI bus 0
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialization Failed!\r\n");
    return;
  } else {
    comms::USB_CDC.printf("SPI Initialization Successful!\r\n");
  }

  // Initialize I2C bus 0
  if (!i2c_init(i2c0, 400 * 1000)) {
    comms::USB_CDC.printf("I2C Initialization Failed!\r\n");
    return;
  } else {
    comms::USB_CDC.printf("I2C Initialization Successful!\r\n");
  }

  // create ultrasound objects
  for (int i = 0; i < 16; i++) {
    ultrasound_group[i] = new Ultrasound();
  }

  // group init
  if (!Ultrasound::group_init()) {
    comms::USB_CDC.printf("Ultrasound group init failed\r\n");
    return;
  }

  // init ultrasounds
  for (int i = 1; i <= 1; i++) {
    ultrasound_group[i] = new Ultrasound();
    if (!ultrasound_group[i]->init(i)) {
      comms::USB_CDC.printf("Ultrasound %d init failed\r\n", i);
      return;
    }
  }

  // group start
  if (!Ultrasound::group_start()) {
    comms::USB_CDC.printf("Ultrasound group start failed\r\n");
    return;
  }
}

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
