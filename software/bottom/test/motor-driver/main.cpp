#include "DRV8244.hpp"
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
// #include "pin_selector.hpp"
// #include "pins/digital_pins.hpp"

MotorDriver driver;
// Pins::DigitalPins digital_pins;

int main() {
  stdio_init_all();
  while (!stdio_usb_connected())
    continue;

  // digital_pins.init();
  driver.init(-1, 1000000); 

  // digital_pins.attach_interrupt(DriverDbgPinMap::NFAULT, Pins::DigitalPinInterruptState::EDGE_FALL, driver.handle_error , &driver);
  // digital_pins.enable_interrupt(DriverDbgPinMap::NFAULT);

  for (int i = 0; i <= 625; i++) {
    driver.command(i * 10);
    sleep_ms(100);
  }
  return 0;
}