#include "comms/usb.hpp"
#include "class/cdc/cdc_device.h"
#include "errors.hpp"
#include "identifiers.hpp"
#include "types.hpp"
extern "C" {
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <tusb.h>
}

using namespace types;

void tud_cdc_rx_cb(uint8_t itf) {}

namespace usb {

CurrentRecvState state;

CDC::CDC() {}

bool CDC::init(void) {
  if (!stdio_usb_init()) {
    return false; // some error, idk
  }
  // usb-cdc initialised, proceed
  _current_recv_state.reset();
  _current_recv_state.data_buffer = _read_buffer;
  // set callback for data avail, to recieve full raw commands async
  _set_data_avail_callback(_data_avail_callback, (void *)&_current_recv_state);
  _is_initialised = true;
}

bool CDC::send_data(const comms::SendIdentifiers identifier, const u8 *data,
                    const u16 data_len) {
  if (data_len == 0xFFFF) {
    return false;
  }
  u16 real_len = data_len + sizeof(identifier);
  write((u8 *)&real_len, sizeof(real_len));     // length bytes
  write((u8 *)&identifier, sizeof(identifier)); // identifier
  write(data, data_len);                        // data
  flush();
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

void CDC::_set_data_avail_callback(void (*function)(void *), void *args) {
  stdio_set_chars_available_callback(function, args);
}

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
