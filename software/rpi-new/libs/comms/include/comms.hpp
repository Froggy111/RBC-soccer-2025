#pragma once
#include "comms/usb.hpp"
#include "types.hpp"

/**
 * INFO:
 * IMPORTANT: Raspberry Pi and Pico are all little endian (least significant byte first)
 * Communication format is defined as such, in both directions:
 * Byte 1 & 2: Length (least significant byte first) (excludes length bytes)
 * Byte 3: Identifier (u8 enum)
 * The rest is passed to the specific handler.
 */

namespace comms {

extern usb::CDC USB_CDC;

} // namespace comms