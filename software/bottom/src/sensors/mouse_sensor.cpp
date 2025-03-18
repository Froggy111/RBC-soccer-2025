#include "ALSPT19.hpp"
#include "PMW3360.hpp"
#include "comms.hpp"
#include "comms/usb.hpp"

MouseSensor mouse_sensor_1, mouse_sensor_2;

void line_sensor_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);

  mouse_sensor_1.init(1, spi0);
  mouse_sensor_2.init(2, spi0);

  while (true) {
    TickType_t previous_wait_time = xTaskGetTickCount();
    
    for (int i = 0; i <= LINE_SENSOR_COUNT; i++) {
      line_sensors.read_raw(0);
    }
    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(2));
  }
}