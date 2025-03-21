#include "PMW3360.hpp"
#include "comms.hpp"
#include "config.hpp"

mouse::MouseSensor mouse_sensor_1, mouse_sensor_2;

void mouse_sensor_task(void *args) {
  mouse_sensor_1.init(1, spi0);
  mouse_sensor_2.init(2, spi0);

  while (true) {
    TickType_t previous_wait_time = xTaskGetTickCount();

    // TODO: Read the data from the mouse sensors

    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(POLL_RATE));
  }
}