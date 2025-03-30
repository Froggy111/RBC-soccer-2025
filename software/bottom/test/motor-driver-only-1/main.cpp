#include "DRV8244.hpp"
#include "comms/usb.hpp"
#include "pins.hpp"

extern "C" {
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <hardware/i2c.h>
}
#include "comms.hpp"

driver::MotorDriver motor_driver;

const int LED_PIN = 25;

void motor_driver_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  pins::digital_pins.init();

  // init as debug
  if (motor_driver.init(4, spi0)) {
    comms::USB_CDC.printf("Motor Driver Initialized!\n");
  } else {
    comms::USB_CDC.printf("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  while (true) {
    for (int i = 0; i <= 625; i++) {
      if (!motor_driver.command(i * 10))
        continue;

      // comms::USB_CDC.printf("Motor Driver Current: %d\n",
      //                       motor_driver.read_current());
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    for (int i = 625; i >= -625; i--) {
      if (!motor_driver.command(i * 10))
        continue;

      // comms::USB_CDC.printf("Motor Driver Current: %d\n",
      //                       motor_driver.read_current());
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    for (int i = -625; i <= 0; i++) {
      if (!motor_driver.command(i * 10))
        continue;

      // comms::USB_CDC.printf("Motor Driver Current: %d\n",
      //                       motor_driver.read_current());
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
