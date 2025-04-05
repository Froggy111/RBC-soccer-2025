#include "comms/identifiers.hpp"
#include "comms/usb.hpp"
extern "C" {
#include <pico/stdlib.h>
#include <hardware/spi.h>
#include <hardware/i2c.h>
}
#include "comms.hpp"
#include "sensors/lightgate.hpp"
#include "sensors/mouse_sensor.hpp"
#include "sensors/line_sensor.hpp"
#include "actions/kicker.hpp"
#include "actions/motors.hpp"
#include "hardware/watchdog.h"

#define LED_PIN 25

TaskHandle_t main_task_handle = nullptr;

void watchdog_task(void *pvParameters) {
  while (1) {
    // Update/feed the watchdog to prevent reset
    watchdog_update();

    // Wait for some time before next update
    // Make sure this is less than your watchdog timeout
    vTaskDelay(pdMS_TO_TICKS(50)); // 50ms delay
  }
}

void main_task(void *args) {
  // * Init LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  debug::info("USB CDC connected.\r\n");

  if (watchdog_enable_caused_reboot()) {
    debug::warn("Crash detected!!!! Watchdog enable caused reboot!\r\n");
  }

  // * Init SPIs
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.write(comms::SendIdentifiers::SPI_INIT_FAIL, NULL, 0);
    vTaskDelete(main_task_handle);
  } else {
    debug::info("SPI0 initialized.\r\n");
  }

  motor_data_mutex = xSemaphoreCreateMutex();
  kicker_mutex = xSemaphoreCreateMutex();

  xTaskCreate(line_sensor_task, "line_sensor_task", 1024, NULL, 10, NULL);
  xTaskCreate(mouse_sensor_task, "mouse_sensor_task", 1024, NULL, 10, NULL);
  xTaskCreate(motor_task, "motor_task", 1024, NULL, 10, &motor_task_handle);
  xTaskCreate(kicker_task, "kicker_task", 1024, NULL, 10, &kicker_task_handle);

  bool motor_attach_successful = comms::USB_CDC.attach_listener(
      comms::RecvIdentifiers::MOTOR_DRIVER_CMD, motor_task_handle,
      motor_data_mutex, motor_task_buffer, sizeof(motor_task_data));

  bool kicker_attach_successful = comms::USB_CDC.attach_listener(
      comms::RecvIdentifiers::KICKER_CMD, kicker_task_handle, kicker_mutex,
      kicker_task_buffer, sizeof(kicker_task_data));

  if (!motor_attach_successful || !kicker_attach_successful) {
    comms::USB_CDC.write(comms::SendIdentifiers::COMMS_ERROR, NULL, 0);
    vTaskDelete(main_task_handle);
  }

  debug::info("All tasks created.\n");

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(portMAX_DELAY));
  }
}

int main() {
  watchdog_enable(100, 1);
  xTaskCreate(watchdog_task, "watchdog_task", configMINIMAL_STACK_SIZE, NULL,
              configMAX_PRIORITIES - 1, NULL);
  // * Init USB Comms
  comms::init();

  xTaskCreate(main_task, "main_task", 1024, NULL, 10, &main_task_handle);

  vTaskStartScheduler();
  while (true)
    ;
}
