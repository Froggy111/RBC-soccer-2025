#include "comms/usb.hpp"
#include "types.hpp"

extern "C" {
#include <FreeRTOS.h>
#include <pico/stdio.h>
}

using namespace types;

int main() {
  usb::CDC cdc = usb::CDC();
  cdc.init();
  u8 payload[] = "Hello world!\n";
  while (true) {
    cdc.write((comms::SendIdentifiers)payload[0], &payload[1],
              sizeof(payload) - 1);
    sleep_ms(1000);
  }
  return 0;
}
