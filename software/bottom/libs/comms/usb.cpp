#include "libs/comms/usb.hpp"
#include "libs/comms/identifiers.hpp"
#include "libs/utils/types.hpp"
#include <hardware/gpio.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <type_traits>

using namespace types;

namespace usb {

CDC::CDC() {}

bool CDC::init(void) {
  if (!stdio_usb_init()) {
    return false; // some error, idk
  }
  // usb-cdc initialised, proceed
  // set callback for data avail, to recieve full raw commands async
  _set_data_avail_callback(_data_avail_callback, (void *)&_current_recv_state);
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
    // sent command is too long, report error
    if (crs->expected_length > max_recieve_buf_size) {
      send_data(comms::SendIdentifiers::ERROR, command_too_long_err,
                sizeof(command_too_long_err));
      crs->reset();
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
      // raise PARITY_ERROR
    }
    RawCommand raw_command = {crs->recieved_length, crs->data_buffer};
    crs->reset();
  }
}

} // namespace usb
