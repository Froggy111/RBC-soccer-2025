#include "comms.hpp"
#include "types.hpp"
#include <hardware/gpio.h>
//#include "PMW3360.hpp"

const types::u8 LED_PIN = 25;

//MouseSensor sensor;


// void mouse_sensor_task(void *args) {
//   if (!spi_init(spi0, 1000000)) {
//     comms::USB_CDC.printf("SPI Initialization Failed!\n");
//   }

//   sensor.init(-1, spi0);
//   while (true) {
//     gpio_put(LED_PIN, 1);
//     vTaskDelay(pdMS_TO_TICKS(500));
//     sensor.write8(0x02, 0, -1);

//     comms::USB_CDC.printf("Motion detected? %d", (sensor.read8(0x02)));
//     gpio_put(LED_PIN, 0);
//   }
//   //return;
//   // for (int i = 0; i < 25; i++) {
//   //     sleep_ms(1000);
//   //     sensor.read_motion_burst();
//   //     types::u16 x_delta, y_delta, shutter;
//   //     for(int j = 0;j < 12; j++){
//   //         cdc->printf("TERM %d: %d \n", j, sensor.motion_burst_buffer[j]);
//   //     }
//   //     x_delta = ((sensor.motion_burst_buffer[2] << 8) | sensor.motion_burst_buffer[3]);
//   //     y_delta = ((sensor.motion_burst_buffer[4] << 8) | sensor.motion_burst_buffer[5]);

//   //     //printf("X_L: %d | X_H: %d \n", sensor.motion_burst_buffer[2], sensor.motion_burst_buffer[3]);
//   //     //printf("Y_L: %d | Y_H: %d \n", sensor.motion_burst_buffer[4], sensor.motion_burst_buffer[5]);

//   //     //printf("X delta: %d | Y delta: %d \n", x_delta, y_delta);

//   //     //printf("Raw Data Sum: %d \n", sensor.motion_burst_buffer[7]);
//   //     //printf("Minimum Raw Data: %d | Maximum Raw Data: %d \n", sensor.motion_burst_buffer[8], sensor.motion_burst_buffer[9]);

//   //     //printf("Shutter Lower: %d | Shutter Upper %d \n", sensor.motion_burst_buffer[10], sensor.motion_burst_buffer[11]);

//   //     shutter = ((sensor.motion_burst_buffer[10] << 8) | sensor.motion_burst_buffer[11]);

//   //     //printf("Shutter Value: %d \n", shutter);
//   // }
// }

// void led_test(void *args){
//     while(true){
//         gpio_put(LED_PIN, 1);
//         vTaskDelay(pdMS_TO_TICKS(500));
//         gpio_put(LED_PIN, 0);
//     }
// }

int main() {

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);
  
//   comms::USB_CDC.init();

//   xTaskCreate(mouse_sensor_task, "mouse_sensor_task", 1024, nullptr, 10, NULL);

//   vTaskStartScheduler();

//   comms::USB_CDC.init();

//   xTaskCreate(led_test, "led_task", 1024, nullptr, 10, NULL);

//   vTaskStartScheduler();

  return 0;
}