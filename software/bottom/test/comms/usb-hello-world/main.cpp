#include "libs/comms/usb.hpp"
#include "libs/utils/types.hpp"
#include <pico/stdio.h>
#include <stdio.h>

using namespace types;

int main() {
  usb::CDC cdc = usb::CDC();
  cdc.init();
  u8 payload[] = "Hello world!\n";
  while (true) {
    cdc.write(payload, sizeof(payload));
    sleep_ms(1000);
  }
  return 0;
}
