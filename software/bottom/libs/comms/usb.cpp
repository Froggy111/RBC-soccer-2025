#include "libs/comms/usb.hpp"
#include "libs/utils/types.hpp"
#include <pico/stdio.h>
#include <pico/stdlib.h>

using namespace types;

namespace usb {

CDC::CDC() {}

bool CDC::init(void) { return stdio_usb_init(); }

void CDC::write(const u8 *data, u32 len) {
  stdio_put_string((char *)data, len, false, false);
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
} // namespace usb
