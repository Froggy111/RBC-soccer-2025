#include "ALSPT19.hpp"
#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "config.hpp"

LineSensor line_sensors;

void line_sensor_task(void *args) {
  line_sensors.init(spi0);
  for (;;) {
    TickType_t previous_wait_time = xTaskGetTickCount();

    uint16_t raw_data[LINE_SENSOR_COUNT];
    for (int i = 0; i < LINE_SENSOR_COUNT; i++) {
      raw_data[i] = line_sensors.read_raw(i);
    }

    comms::USB_CDC.write(comms::SendIdentifiers::LINE_SENSOR_DATA,
                          reinterpret_cast<uint8_t *>(raw_data),
                          sizeof(raw_data));
                          
    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(POLL_RATE));
  }
}