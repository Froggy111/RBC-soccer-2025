#include "comms/identifiers.hpp"
extern "C" {
#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <hardware/i2c.h>
}
#include "comms.hpp"
#include "sensors/IMUs.hpp"
#include "actions/LEDs.hpp"

static const types::u8 LED_PIN = 25;

void main_task(void *params) {
  comms::USB_CDC.wait_for_CDC_connection(portMAX_DELAY);
  debug::info("Initialising SPI\r\n");
  if (!spi_init(spi0, 4e6)) {
    debug::fatal("SPI initialisation failed\r\n");
  }
  debug::info("Creating IMU poll task\r\n");
  xTaskCreate(imu_poll_task, "imu_poll_task", 1024, NULL, 10,
              &imu_poll_task_handle);
  debug::info("Created all tasks\r\n");
  while (true) {
    vTaskDelay(portMAX_DELAY);
  }
}

int main() {
  // * Init LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // * Init USB Comms
  comms::init();
  xTaskCreate(main_task, "main task", 1024, nullptr, 12, nullptr);

  vTaskStartScheduler();
}
