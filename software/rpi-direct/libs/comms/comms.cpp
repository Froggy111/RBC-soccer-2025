#include "comms.hpp"
#include "comms/uart.hpp"
#include "comms/usb.hpp"
#include "types.hpp"
#include <iostream>

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

bool comms_init(const std::string& usb_device_path) {
  // Initialize USB CDC
  if (!USB_CDC.init(usb_device_path)) {
    std::cerr << "Failed to initialize USB CDC with device: " << usb_device_path << std::endl;
    return false;
  }
  
  // Wait for device connection
  if (!USB_CDC.wait_for_device_connection()) {
    std::cerr << "Failed to connect to USB device" << std::endl;
    return false;
  }
  
  // UART initialization can be added later
  
  return true;
}

} // namespace comms