#include "DRV8244.hpp"
#include "comms/usb.hpp"
#include "debug.hpp"
#include "projdefs.h"

extern "C" {
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <hardware/i2c.h>
}
#include "comms.hpp"

driver::MotorDriver motor_driver;

const int LED_PIN = 25;
TickType_t xLastClearFaultTime = 0;

void motor_driver_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  // init as debug
  if (motor_driver.init(1, spi0)) {
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

// void motor_driver_clear_fault_task(void *args) {
//   comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);
//   vTaskDelay(pdMS_TO_TICKS(100));
//   while (true) {
//     xLastClearFaultTime = xTaskGetTickCount();
//     vTaskDelayUntil(&xLastClearFaultTime, pdMS_TO_TICKS(500));
//     motor_driver.clear_fault();
//     debug::debug("Fault cleared!\n");
//   }
// }

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::init();
  xTaskCreate(motor_driver_task, "motor_driver_task", 1024, NULL, 10, NULL);
  // xTaskCreate(motor_driver_clear_fault_task, "motor_driver_clear_fault_task",
  //             1024, NULL, 10, NULL);

  vTaskStartScheduler();
}
