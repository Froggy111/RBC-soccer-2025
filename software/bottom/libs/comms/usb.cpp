#include "comms/usb.hpp"
#include "class/cdc/cdc_device.h"
#include "comms/errors.hpp"
#include "comms/identifiers.hpp"
#include "device/usbd.h"
#include "types.hpp"
#include <hardware/timer.h>
extern "C" {
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <tusb.h>
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
}

using namespace types;

namespace usb {

CurrentRXState state;

CDC::CDC() {}

bool CDC::init(void) {
  if (tusb_init()) {
    return false; // some error, idk
  }
  // usb-cdc initialised, proceed
  _current_recv_state.reset();
  _current_recv_state.data_buffer = _read_buffer;
  _is_initialised = true;
}

bool CDC::wait_for_host_connection(u32 timeout) {
  u64 t_start = time_us_64();
  do {
  } while (tud_connected() && time_us_64() - t_start < timeout);
  if (tud_connected()) {
    return true;
  }
  return false;
}

bool CDC::wait_for_CDC_connection(u32 timeout) {
  u64 t_start = time_us_64();
  do {
  } while (tud_cdc_connected() && time_us_64() - t_start < timeout);
  if (tud_cdc_connected()) {
    return true;
  }
  return false;
}

bool CDC::write(const comms::SendIdentifiers identifier, const u8 *data,
                const u16 data_len) {
  u16 reported_len = data_len + sizeof(identifier);
  u16 packet_len = sizeof(reported_len) + reported_len;
  if (packet_len > MAX_TRANSMIT_BUF_SIZE) {
    return false;
  }
  if (tud_cdc_available() < packet_len) {
    tud_cdc_write_flush(); // NOTE: this blocks (in tinyusb + rp2040), which we want.
  }
  // write to tusb CDC buffer
  tud_cdc_write(&reported_len, sizeof(reported_len));
  tud_cdc_write(&identifier, sizeof(identifier));
  tud_cdc_write(data, data_len);

  return true;
}

u32 CDC::read(u8 *data, u32 len, u32 timeout) {
  u64 t_start = time_us_64();
  u32 n_read = 0;

  while (time_us_64() - t_start < timeout) {
    int read = stdio_getchar();
    if (read < 0) { // EOF or some other weird thing
      continue;
    }
    data[n_read] = (u8)read;
    n_read++;
  }
}

void CDC::_line_coding_cb(u8 interface, cdc_line_coding_t const *coding,
                          void *args) {
  // Handle reset via baud rate if needed
  if (coding->bit_rate == 1200) {
    reset_usb_boot(0, 0);
  }
}

bool CDC::_vendor_control_xfer_cb(u8 rhport, u8 stage,
                                  tusb_control_request_t const *request,
                                  void *args) {
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

void CDC::_rx_cb(u8 interface, void *args) {}

void CDC::_data_avail_callback(void *args) {
  CurrentRecvState *crs = (CurrentRecvState *)args;
  u8 newbyte = stdio_getchar();
  // length bytes
  if (crs->length_bytes_recieved < 2) {
    crs->expected_length =
        newbyte << (8 * crs->length_bytes_recieved); // assuming little endian
    crs->length_bytes_recieved++;
    // sent command is too long, report error
    if (crs->expected_length > max_recieve_buf_size) {
      comms::Errors err = comms::Errors::PACKET_RECV_TOO_LONG;
      // FIXME: This will not work, stdio cannot be used in this callback
      send_data(comms::SendIdentifiers::COMMS_ERROR, (u8 *)&err, sizeof(err));
      crs->reset();
      // NOTE: after this, behavior becomes undefined
    }
    return;
  }
  // data bytes
  if (crs->recieved_length < crs->expected_length) {
    crs->data_buffer[crs->recieved_length] = newbyte;
    crs->recieved_length++;
    crs->parity_byte ^= newbyte;
    return;
  }
  // last byte (parity byte)
  else if (crs->recieved_length == crs->expected_length) {
    // check parity
    if (crs->parity_byte != newbyte) {
      // raise PARITY_FAILED
      comms::Errors err = comms::Errors::PARITY_FAILED;
      // FIXME: This will not work, stdio cannot be used in this callback
      send_data(comms::SendIdentifiers::COMMS_ERROR, (u8 *)&err, sizeof(err));
    }
    RawCommand raw_command = {crs->recieved_length, crs->data_buffer};
    crs->reset();
  }
  // NOTE: SOMETHING REALLY WENT WRONG
  else {
    comms::Errors err = comms::Errors::RECIEVED_MORE_THAN_EXPECTED;
    // FIXME: This will not work, stdio cannot be used in this callback
    send_data(comms::SendIdentifiers::COMMS_ERROR, (u8 *)&err, sizeof(err));
  }
}

} // namespace usb
