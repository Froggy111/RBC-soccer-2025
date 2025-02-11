#pragma once

#include "libs/utils/types.hpp"
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <tusb.h>

namespace usb {

class CDC {
public:
  CDC();
  bool enable_usb_boot(void);  // this should always be called, probably
  bool disable_usb_boot(void); // this should never be called, probably

  bool init_tud_cdc(void);

  inline void tusb_write(types::u8 *data, types::u32 len) {
    _tusb_write(data, len);
  }
  inline types::u32 tusb_read(types::u8 *data, types::u32 maxlen) {
    _tusb_read(data, maxlen);
  }

private:
  bool _usb_boot_enabled = false;
  types::u32 _baudrate;
  /**
   * @brief write data from buffer
   * @param data: u8 array
   * @param len: length of data
   * @returns whether write was successful
   */
  void _tusb_write(types::u8 *data, types::u32 len);
  /**
   * @brief read data into buffer with some max length
   * @param data: u8 array
   * @param maxlen: max length permitted to be read
   * @note does NOT handle null terminator!
   * @returns length of read data
   */
  types::u32 _tusb_read(types::u8 *data, types::u32 maxlen);
};

} // namespace usb
