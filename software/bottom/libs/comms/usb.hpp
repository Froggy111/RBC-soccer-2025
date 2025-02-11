#pragma once

#include "libs/utils/types.hpp"
#include <pico/stdio.h>
#include <pico/stdlib.h>

namespace usb {

class CDC {
public:
  CDC();
  bool init(void);

  /**
   * @brief write data from buffer
   * @param data: u8 array
   * @param len: length of data
   */
  void write(const types::u8 *data, types::u32 len);
  /**
   * @brief read data into buffer with some max length
   * @param data: u8 array
   * @param len: length to be read
   * @param timeout: max time spent in this function, in millis
   * @note does NOT care about null terminator!
   * @returns how many characters read
   */
  types::u32 read(types::u8 *data, types::u32 len, types::u32 timeout);

private:
};

} // namespace usb
