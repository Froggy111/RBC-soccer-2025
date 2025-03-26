#pragma once
#include "comms/uart.hpp"
#include "comms/usb.hpp"
#include "types.hpp"
#include <string>

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

/**
 * @brief initialises communication interfaces
 * @param usb_device_path path to the USB device (e.g., "/dev/ttyACM0")
 * @returns true if successfully initialized, false otherwise
 */
bool comms_init(const std::string& usb_device_path = "/dev/ttyACM0");

} // namespace comms