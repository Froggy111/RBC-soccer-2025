#include "comms.hpp"
#include "projdefs.h"
#include "types.hpp"
#include <cstddef>
#include <hardware/gpio.h>
#include <pico/time.h>
#include "PMW3360.hpp"

const types::u8 LED_PIN = 25;

MouseSensor sensor;

void led_blink_task(void *args){
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(LED_PIN, 1);
    }
}

void mouse_sensor_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xffffffff);
  comms::USB_CDC.printf("CDC Connected!\r\n");

  //usb::CDC *cdc = (usb::CDC *)args;
  if (!spi_init(spi0, 1000000)) {
    comms::USB_CDC.printf("SPI Initialization Failed!\r\n");
    return;
  } else {
    comms::USB_CDC.printf("SPI Initialization Successful!\r\n");
  }

  if (!sensor.init(1, spi0)) {
    comms::USB_CDC.printf("Mouse Sensor Initialization Failed!\r\n");
    return;
  } else {
    comms::USB_CDC.printf("Mouse Sensor Initialised!\r\n");
  }
  
  while (true) {
      comms::USB_CDC.printf("====================================\r\n");

      sleep_ms(500);
      sensor.read_motion_burst();

      types::u16 x_delta, y_delta, shutter;
      for(int j = 0;j < 12; j++){
        comms::USB_CDC.printf("TERM %u: %u \r\n", j, sensor.motion_burst_buffer[j]);
      }

      x_delta = ((sensor.motion_burst_buffer[2] << 8) | sensor.motion_burst_buffer[3]);
      y_delta = ((sensor.motion_burst_buffer[4] << 8) | sensor.motion_burst_buffer[5]);
      comms::USB_CDC.printf("X_L: %u | X_H: %u \r\n", sensor.motion_burst_buffer[2], sensor.motion_burst_buffer[3]);
      comms::USB_CDC.printf("Y_L: %u | Y_H: %u \r\n", sensor.motion_burst_buffer[4], sensor.motion_burst_buffer[5]);
      comms::USB_CDC.printf("X delta: %u | Y delta: %u \r\n", x_delta, y_delta);
  }
}

int main() {

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.init();

  xTaskCreate(led_blink_task, "led_blink_task", 1024, NULL, 10, NULL);
  xTaskCreate(mouse_sensor_task, "mouse_sensor_task", 1024, NULL, 10, NULL);
  vTaskStartScheduler();

  return 0;
}