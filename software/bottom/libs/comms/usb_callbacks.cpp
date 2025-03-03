#include "comms/usb.hpp"
#include "types.hpp"

extern "C" {
#include "tusb.h"
}

using namespace types;

namespace usb {

CDCLineCodingCB CDC_line_coding_cb_fn = nullptr;
void *CDC_line_coding_cb_user_args = nullptr;

VendorControlXferCB vendor_control_xfer_cb_fn = nullptr;
void *vendor_control_xfer_cb_user_args = nullptr;

CDCRxCB CDC_rx_cb_fn = nullptr;
void *CDC_rx_cb_user_args = nullptr;

MountCB mount_cb_fn = nullptr;
void *mount_cb_user_args = nullptr;

CDCLineStateCB CDC_line_state_cb_fn = nullptr;
void *CDC_line_state_cb_user_args = nullptr;
} // namespace usb

// WARN: These assume that the function and arg pointers are set before tusb_init().
// CDC callbacks
void tud_cdc_line_coding_cb(u8 itf, cdc_line_coding_t const *coding) {
  usb::CDC_line_coding_cb_fn(itf, coding, usb::CDC_line_coding_cb_user_args);
}
void tud_cdc_line_state_cb(u8 itf, bool dtr, bool rts) {
  usb::CDC_line_state_cb_fn(itf, dtr, rts, usb::CDC_line_state_cb_user_args);
}
void tud_cdc_rx_cb(u8 itf) { usb::CDC_rx_cb_fn(itf, usb::CDC_rx_cb_user_args); }

// Vendor interface callbacks for reset
bool tud_vendor_control_xfer_cb(u8 rhport, u8 stage,
                                tusb_control_request_t const *request) {
  return usb::vendor_control_xfer_cb_fn(rhport, stage, request,
                                        usb::vendor_control_xfer_cb_user_args);
}

void tud_mount_cb(void) { usb::mount_cb_fn(usb::mount_cb_user_args); }
