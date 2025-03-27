#include "comms.hpp"
#include "comms/usb.hpp"
#include "types.hpp"

namespace comms {

usb::CDC USB_CDC;

bool comms_init(void) {
    return USB_CDC.init();
}

std::vector<usb::USBDevice> get_connected_devices() {
    return USB_CDC.scan_devices();
}

usb::USBDevice get_device_by_type(usb::DeviceType type) {
    auto devices = USB_CDC.scan_devices();
    for (const auto& device : devices) {
        if (device.type == type) {
            usb::USBDevice connected_device = device;
            if (USB_CDC.connect(connected_device)) {
                return connected_device;
            }
        }
    }
    return usb::USBDevice(); // Return empty device if not found
}

} // namespace comms