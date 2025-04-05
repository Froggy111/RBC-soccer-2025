#include "ALSPT19.hpp"
#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "config.hpp"

LineSensor line_sensors;

const types::u8 channel_count = LINE_SENSOR_COUNT / 3;

void line_sensor_task(void *args) {
  line_sensors.init(spi0);
  for (;;) {
    TickType_t previous_wait_time = xTaskGetTickCount();

    uint16_t raw_data[LINE_SENSOR_COUNT];
    for (int i = 0; i < channel_count; i++) {
      raw_data[i] = line_sensors.read_raw(i);
      raw_data[i + channel_count] = line_sensors.read_raw(i + channel_count);
      raw_data[i + channel_count * 2] =
          line_sensors.read_raw(i + channel_count * 2);
    }

    comms::USB_CDC.write(comms::SendIdentifiers::LINE_SENSOR_DATA,
                         reinterpret_cast<uint8_t *>(raw_data),
                         sizeof(raw_data));

    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(POLL_RATE));
  }
}
