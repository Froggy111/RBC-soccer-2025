#include "ALSPT19.hpp"
#include "comms.hpp"
#include "comms/usb.hpp"
#define LINE_SENSOR_COUNT 48

LineSensor line_sensors;

void line_sensor_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  line_sensors.init(spi0);

  while (true) {
    TickType_t previous_wait_time = xTaskGetTickCount();
    
    for (int i = 0; i <= LINE_SENSOR_COUNT; i++) {
      line_sensors.read_raw(0);
    }
    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(100));
  }
}