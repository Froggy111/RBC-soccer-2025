extern "C" {
    #include <pico/stdio.h>
    #include <pico/stdio_usb.h>
    #include <stdio.h>
}

#include "PMW3360.hpp"

// #include "pin_selector.hpp"
// #include "pins/digital_pins.hpp"

MouseSensor sensor;
// Pins::DigitalPins digital_pins;

int main() {
  stdio_init_all();
  while (!stdio_usb_connected())
    continue;

  sensor.init(-1, 1000000); 

  for(int i = 0; i < 25; i++){
    sleep_ms(1000);
    sensor.read_motion_burst();
    types::u16 x_delta, y_delta, shutter;
    x_delta = ((sensor.motion_burst_buffer[2]<<8) & sensor.motion_burst_buffer[3]);
    y_delta = ((sensor.motion_burst_buffer[4]<<8) & sensor.motion_burst_buffer[5]);

    printf("X_L: %d | X_H: %d \n", sensor.motion_burst_buffer[2], sensor.motion_burst_buffer[3]);
    printf("Y_L: %d | Y_H: %d \n", sensor.motion_burst_buffer[4], sensor.motion_burst_buffer[5]);

    printf("X delta: %d | Y delta: %d \n", x_delta, y_delta);

    printf("Raw Data Sum: %d \n", sensor.motion_burst_buffer[7]);
    printf("Minimum Raw Data: %d | Maximum Raw Data: %d \n", sensor.motion_burst_buffer[8], sensor.motion_burst_buffer[9]);
    
    prinf("Shutter Lower: %d | Shutter Upper %d \n", sensor.motion_burst_buffer[10], sensor.motion_burst_buffer[11]);

    shutter = ((sensor.motion_burst_buffer[10]<<8) & sensor.motion_burst_buffer[11]);

    printf("Shutter Value: %d \n", shutter);
  }

  return 0;
}