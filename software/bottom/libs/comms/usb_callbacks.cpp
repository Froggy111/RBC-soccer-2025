#include "common/tusb_types.h"
#include "comms/usb.hpp"
#include "device/usbd.h"
#include "tusb.h"
#include "types.hpp"

using namespace types;

namespace usb {

CDCLineCodingCB CDC_line_coding_cb_fn = nullptr;
void *CDC_line_coding_cb_user_args = nullptr;

VendorControlXferCB vendor_control_xfer_cb_fn = nullptr;
void *vendor_control_xfer_cb_user_args = nullptr;

CDCRxCB CDC_rx_cb_fn = nullptr;
void *CDC_rx_cb_user_args = nullptr;

} // namespace usb

// CDC callbacks
void tud_cdc_line_coding_cb(u8 itf, cdc_line_coding_t const *coding) {
  usb::CDC_line_coding_cb_fn(itf, coding, usb::CDC_line_coding_cb_user_args);
}

bool tud_vendor_control_xfer_cb(u8 rhport, u8 stage,
                                tusb_control_request_t const *request) {
  return usb::vendor_control_xfer_cb_fn(rhport, stage, request,
                                        usb::vendor_control_xfer_cb_user_args);
}

// Vendor interface callbacks for reset

void tud_cdc_rx_cb(u8 itf)
