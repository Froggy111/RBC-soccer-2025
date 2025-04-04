#include "config.hpp"
#include "comms.hpp"

void mouse_sensor_task(void *args) {
  while (true) {
    TickType_t previous_wait_time = xTaskGetTickCount();

    // TODO: Read the data from the mouse sensors

    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(POLL_RATE));
  }
}