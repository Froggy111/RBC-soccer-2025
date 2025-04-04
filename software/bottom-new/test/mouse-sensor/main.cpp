#include "comms.hpp"
#include "comms/usb.hpp"
#include "projdefs.h"
#include "types.hpp"
#include <hardware/gpio.h>
#include <pico/time.h>
#include "PMW3360.hpp"
#include "debug.hpp"

const types::u8 LED_PIN = 25;

mouse::MouseSensor sensor;

int urgent_blink() {
  while (true) {
    gpio_put(LED_PIN, 1);
    sleep_ms(100);
    gpio_put(LED_PIN, 0);
    sleep_ms(100);
  }
}

void mouse_sensor_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xffffffff);
  debug::log("CDC Connected!\r\n");

  //usb::CDC *cdc = (usb::CDC *)args;
  if (!spi_init(spi0, 1000000)) {
    debug::log("SPI Initialization Failed!\r\n");
    urgent_blink();
  } else {
    debug::log("SPI Initialization Successful!\r\n");
  }

  if (!sensor.init(1, spi0)) {
    debug::log("Mouse Sensor Initialization Failed!\r\n");
    urgent_blink();
  } else {
    debug::log("Mouse Sensor Initialised!\r\n");
  }

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    sensor.read_motion_burst();

    types::u16 x_delta, y_delta, shutter;
    for (int j = 0; j < 12; j++) {
      debug::log("TERM %u: %u \r\n", j,
                            sensor.motion_burst_buffer[j]);
    }

    x_delta =
        ((sensor.motion_burst_buffer[2] << 8) | sensor.motion_burst_buffer[3]);
    y_delta =
        ((sensor.motion_burst_buffer[4] << 8) | sensor.motion_burst_buffer[5]);
    debug::log("X_L: %u | X_H: %u \r\n",
                          sensor.motion_burst_buffer[2],
                          sensor.motion_burst_buffer[3]);
    debug::log("Y_L: %u | Y_H: %u \r\n",
                          sensor.motion_burst_buffer[4],
                          sensor.motion_burst_buffer[5]);
    debug::log("X delta: %u | Y delta: %u \r\n", x_delta, y_delta);
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::init();

  xTaskCreate(mouse_sensor_task, "mouse_sensor_task", 1024, NULL, 10, NULL);
  vTaskStartScheduler();

  return 0;
}