#include "libs/comms/usb.hpp"
#include "libs/utils/types.hpp"

using namespace types;

int main() {
  usb::CDC cdc = usb::CDC();
  cdc.enable_usb_boot();
  cdc.init_tud_cdc();

  u8 payload[] = "Hello world!";
  while (true) {
    cdc.tusb_write(payload, sizeof(payload));
    sleep_ms(1000);
  }
}
