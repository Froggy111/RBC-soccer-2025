#include "DRV8244.hpp"
#include "comms/usb.hpp"

extern "C" {
#include <pico/stdlib.h>
#include <hardware/spi.h>
}
#include "comms.hpp"

driver::MotorDriver driver1, driver2, driver3, driver4;

const int LED_PIN = 25;

void motor_driver_task(void *args) {
vTaskDelay(pdMS_TO_TICKS(15000));
  
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialization Failed!\n");
  }

  // init as debug
  driver1.init(1, spi0);
  driver2.init(2, spi0);
  driver3.init(3, spi0);
  driver4.init(4, spi0);

  while (true) {
    for (int i = 0; i <= 1050; i++) {
      if (!driver1.command(-i * 10))
        break;
      busy_wait_us(2);
      if (!driver2.command(i * 10))
        break;
      busy_wait_us(2);
      if (!driver3.command(i * 10))
        break;
      busy_wait_us(2);
      if (!driver4.command(i * 10))
        break;
      busy_wait_us(2);
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    for (int i = 1050; i >= 0; i--) {
      if (!driver1.command(-i * 10))
        break;
      busy_wait_us(2);
      if (!driver2.command(i * 10))
        break;
      busy_wait_us(2);
      if (!driver3.command(i * 10))
        break;
      busy_wait_us(2);
      if (!driver4.command(i * 10))
        break;
      busy_wait_us(2);
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.init();
  xTaskCreate(motor_driver_task, "motor_driver_task", 1024, NULL, 10, NULL);

  vTaskStartScheduler();
}
