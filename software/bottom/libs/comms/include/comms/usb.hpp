#pragma once

/**
 * NOTE:
 * Uses the pico_stdio_usb drivers to handle usb-cdc communication while preserving the usb boot interface.
 * This is not ideal, as the drivers don't expose lower level APIs.
 * It would be more performant and maybe lower latency to reimplement this ourselves.
 * Not urgent though, probably does not matter by too much.
 */

#include "comms/default_usb_config.h"
#include "identifiers.hpp"
#include "types.hpp"
extern "C" {
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
}

/**
 * WARNING: RP2040 and RPi are all little endian (least significant byte first)
 * INFO:
 * Communication format is defined as such, in both directions:
 * Byte 1 & 2: Length (least significant byte first) (excludes length bytes)
 * (pico will check that length <= max_recieve_buf_size)
 * WARNING: LENGTH INCLUDES IDENTIFIER BYTE. This is done for convenience in the recieving state.
 * INFO:
 * Byte 3: Identifier (u8 enum)
 * The rest is passed to the specific handler.
 */
namespace usb {

static const types::u16 max_recieve_buf_size =
    USB_RX_BUFSIZE; // this should absolutely be enough for all recieved commands

struct CurrentRXState {
  types::u8 length_bytes_recieved = 0;
  types::u16 expected_length = 0;
  types::u16 recieved_length = 0;
  types::u8 *data_buffer = nullptr;
  inline void reset(void) {
    length_bytes_recieved = 0;
    expected_length = 0;
    recieved_length = 0;
    memset(0, data_buffer, max_recieve_buf_size);
  }
};

struct RawCommand {
  types::u16 command_length = 0;
  types::u8 *data_buffer = nullptr;
};

class CDC {
public:
  CDC();
  bool init(void);
  inline bool is_initialised(void) { return _is_initialised; }
  /**
   * @brief write data from buffer
   * @param data: u8 array
   * @param len: length of data
   */
  static void write(const types::u8 *data, const types::u16 len);
  /**
   * @brief flush write buffer
   */
  static void flush(void);
  /**
   * @brief sends data, formatted correctly, will flush buffer.
   * @param idenfitier: identifier for the sent data
   * @param data: u8 array
   * @param data_len: length of data
   * @returns true if successfully sent, false if not
   */
  static bool send_data(const comms::SendIdentifiers identifier,
                        const types::u8 *data, const types::u16 data_len);
  /**
   * @brief read data into buffer with some max length
   * @param data: u8 array
   * @param len: length to be read
   * @param timeout: max time spent in this function, in millis
   * @note does NOT care about null terminator!
   * @note this function should not really be used
   * @returns how many characters read
   */
  static types::u32 read(types::u8 *data, types::u32 len, types::u32 timeout);
  /**
   * @brief set callback to run when a command has been fully recieved.
   * This should handle setting flags and triggering events, and should be a very short function.
   * @param function: void function that takes in ptr to RawCommand and void ptr (parse inside the function)
   * @param args: pointer to custom arguments to be passed to callback (struct ptr cast to void ptr)
   */
  static void _set_command_recv_callback(void (*function)(RawCommand *, void *),
                                         void *args);

private:
  /**
   * @brief set callback to run when data is available (new data recieved)
   * @param function: void function with only param as void pointer (parse inside the function).
   * @param args: pointer to arguments to be passed to callback. (struct ptr cast to void ptr)
   */
  static void _set_data_avail_callback(void (*function)(void *), void *args);
  /**
   * @brief callback that adds to data buffer while parsing length. Feeds command into command_recv_callback.
   * @param args: ptr to CurrentRXState (is cast to void ptr due to the stdio callback hook being void ptr).
   */
  static void _data_avail_callback(void *args);
  bool _is_initialised = false;
  types::u8 _read_buffer[max_recieve_buf_size] = {0};
  CurrentRXState _current_recv_state;
};

} // namespace usb
