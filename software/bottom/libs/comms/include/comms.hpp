#pragma once
#include "comms/uart.hpp"
#include "comms/usb.hpp"
#include "types.hpp"

/**
 * INFO:
 * IMPORTANT: RP2040 and RPi are all little endian (least significant byte first)
 * Communication format is defined as such, in both directions:
 * Byte 1 & 2: Length (least significant byte first) (excludes length bytes)
 * Byte 3: Identifier (u8 enum)
 * The rest is passed to the specific handler.
 */

namespace comms {

extern usb::CDC USB_CDC;
extern uart::Serial UART_serial;

// initialises communication interfaces
bool comms_init(void);

} // namespace comms
