// for enabling usb boot
#define PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE 1
#include "libs/comms/usb.hpp"
#include "libs/utils/types.hpp"
#include <class/cdc/cdc_device.h>
#include <device/usbd.h>
#include <tusb.h>

using namespace types;

namespace usb {

CDC::CDC() {}

bool CDC::enable_usb_boot(void) {
  if (stdio_usb_init()) {
    _usb_boot_enabled = true;
    return true;
  } else {
    return false;
  }
}

bool CDC::disable_usb_boot(void) {
  if (stdio_usb_deinit()) {
    _usb_boot_enabled = false;
    return true;
  } else {
    return false;
  }
}

bool CDC::init_tud_cdc(void) { return tusb_init(); }

void CDC::_tusb_write(u8 *data, u32 len) {
  tud_task();

  // write in chunks
  u32 written = 0;
  while (written < len) {
    u32 avail = tud_cdc_write_available();
    if (!avail) {
      tud_task();
      continue;
    }
    u32 to_write = MIN(len - written, avail);
    tud_cdc_write(data, to_write);
    tud_cdc_write_flush();
    written += to_write;
  }
}

u32 CDC::_tusb_read(u8 *data, u32 maxlen) {
  tud_task();
  u32 avail = tud_cdc_available();
  u32 to_read = MIN(maxlen, avail);
  return tud_cdc_read(data, to_read);
}

} // namespace usb
