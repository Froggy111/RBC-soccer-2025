#pragma once

#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <tusb.h>

namespace usb {

class CDC {
public:
  int enable_usb_boot(void);

private:
};

} // namespace usb
