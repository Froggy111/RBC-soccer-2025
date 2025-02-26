#include "DRV8244.hpp"
// #include "pin_selector.hpp"
// #include "pins/digital_pins.hpp"
extern "C" {
  #include <pico/stdlib.h>
  #include <pico/stdio_usb.h>
  #include <hardware/spi.h>
  #include <pico/stdio.h>
  #include <stdio.h>
}

MotorDriver driver;
// Pins::DigitalPins digital_pins;

int main() {
  stdio_init_all();
  while (!stdio_usb_connected())
    continue;

  // digital_pins.init();
  bool result = spi_init(spi0, 1000000);
  printf("SPI init: %d\n", result);
  if (!result) {
    printf("SPI init failed\n");
    return 0;
  }
    
  driver.init(1, spi0);

  // digital_pins.attach_interrupt(DriverDbgPinMap::NFAULT, Pins::DigitalPinInterruptState::EDGE_FALL, driver.handle_error , &driver);
  // digital_pins.enable_interrupt(DriverDbgPinMap::NFAULT);

  // for (int i = 0; i <= 625; i++) {
  //   if (!driver.command(i * 10)) break;
  //   sleep_ms(100);
  // }
  return 0;
}