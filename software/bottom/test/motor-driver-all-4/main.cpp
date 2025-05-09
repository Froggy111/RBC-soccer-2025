#include "DRV8244.hpp"
#include "comms/usb.hpp"
#include "projdefs.h"

extern "C" {
#include <pico/stdlib.h>
#include <hardware/spi.h>
}
#include "comms.hpp"
#include "debug.hpp"

driver::MotorDriver driver1, driver2, driver3, driver4;

const int LED_PIN = 25;

void motor_driver_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  if (!spi_init(spi0, 1000000)) {
    debug::log("SPI Initialization Failed!\n");
  } else {
    debug::log("SPI Initialized!\n");
  }

  // init as debug
  if (driver1.init(1, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver3.init(3, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver4.init(4, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver2.init(2, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  while (true) {
    driver1.command(2000);
    driver2.command(2000);
    driver3.command(2000);
    driver4.command(2000);
    vTaskDelay(pdMS_TO_TICKS(1));
    //   for (int i = 0; i <= 650; i++) {
    //     driver1.command(i * 10);
    //     driver2.command(i * 10);
    //     driver3.command(i * 10);
    //     driver4.command(i * 10);
    //
    //     // debug::log"Current: %d %d %d %d\n", driver1.read_current(), driver2.read_current(),
    //     //        driver3.read_current(), driver4.read_current());
    //
    //     vTaskDelay(pdMS_TO_TICKS(10));
    //   }
    //
    //   vTaskDelay(pdMS_TO_TICKS(10));
    //   for (int i = 650; i >= 0; i--) {
    //     driver1.command(i * 10);
    //     driver2.command(i * 10);
    //     driver3.command(i * 10);
    //     driver4.command(i * 10);
    //
    //     // debug::log"Current: %d %d %d %d\n", driver1.read_current(), driver2.read_current(),
    //     //        driver3.read_current(), driver4.read_current());
    //
    //     vTaskDelay(pdMS_TO_TICKS(10));
    //   }
    //   vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::init();
  xTaskCreate(motor_driver_task, "motor_driver_task", 1024, NULL, 10, NULL);

  vTaskStartScheduler();
}
