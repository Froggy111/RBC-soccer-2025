#include "comms.hpp"
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

usb::CDC USB_CDC = usb::CDC();
uart::Serial UART_serial = uart::Serial();

} // namespace comms
