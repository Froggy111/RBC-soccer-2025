#include "comms/usb.hpp"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "tusb.h"
#include "types.hpp"

namespace usb_cb_hooks {

using tud_cdc_line_coding_cb_t = void (*)(types::u8, const cdc_line_coding_t,
                                          void *);
using tud_vendor_control_xfer_cb_t = void (*)(u8, u8,
                                              const tusb_control_request_t,
                                              void *);

tud_cdc_line_coding_cb_t tud_line_coding_cb;

} // namespace usb_cb_hooks

// CDC callbacks
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding) {
  // Handle reset via baud rate if needed
  if (coding->bit_rate == 1200) {
    reset_usb_boot(0, 0);
  }
}

// Vendor interface callbacks for reset
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request) {
  if (stage != CONTROL_STAGE_SETUP)
    return true;

  switch (request->bRequest) {
  case 0x01: // Reset to bootloader
    reset_usb_boot(0, 0);
    return true;

  case 0x02: // Reset to application
    watchdog_reboot(0, 0, 100);
    return true;
  }

  return false;
}
