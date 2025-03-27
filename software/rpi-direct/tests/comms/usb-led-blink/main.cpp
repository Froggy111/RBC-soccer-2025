#include "comms.hpp"
#include "debug.hpp"
#include <unistd.h>
#include <signal.h>
#include <cstdlib>

// Structure matching the one on the Pico
struct BlinkOnCmdData {
  types::u16 blink_time_ms = 0;
};

bool running = true;

void signal_handler(int signal) {
    running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    
    // Parse command line argument for blink duration
    types::u16 blink_duration = 10000; // Default 500ms
    if (argc > 1) {
        blink_duration = static_cast<types::u16>(std::atoi(argv[1]));
    }
    
    // * Initialize communications
    if (!comms::comms_init()) {
        debug::error("Failed to initialize communications");
        return 1;
    } else {
        debug::info("Communications initialized successfully");
    }
    
    // * Scan for devices
    debug::info("Scanning for devices...");
    std::vector<usb::USBDevice> devices = comms::get_connected_devices();
    if (devices.empty()) {
        debug::error("No Pico devices found");
        return 1;
    }
    
    debug::info("Found %zu devices", devices.size());
    
    // Print out found devices
    for (size_t i = 0; i < devices.size(); i++) {
        debug::info("Device %zu: %s (VID: 0x%04X, PID: 0x%04X)", 
                   i, 
                   devices[i].deviceNode.c_str(), 
                   devices[i].vid, 
                   devices[i].pid);
    }
    
    // * Try to open a connection to each device
    usb::USBDevice connected_device;
    bool connected = false;
    
    for (auto& device : devices) {
        debug::info("Attempting to connect to %s...", device.deviceNode.c_str());
        if (comms::USB_CDC.connect(device)) {
            connected_device = device;
            connected = true;
            debug::info("Connected to %s successfully", device.deviceNode.c_str());
            break;
        }
    }
    
    if (!connected) {
        debug::error("Failed to connect to any Pico device");
        return 1;
    }
    
    // Create the blink command data
    BlinkOnCmdData blink_data;
    blink_data.blink_time_ms = blink_duration;
    
    // Send the command to blink the LED
    debug::info("Sending blink command with duration %u ms", blink_data.blink_time_ms);
    if (comms::USB_CDC.write(connected_device, 
                             comms::SendIdentifiers::DEBUG_TEST_BLINK, 
                             reinterpret_cast<types::u8*>(&blink_data), 
                             sizeof(blink_data))) {
        debug::info("Command sent successfully");
    } else {
        debug::error("Failed to send command");
        return 1;
    }
    
    // Wait for the blink to complete (plus a small buffer)
    debug::info("Waiting for LED to blink...");
    usleep((blink_data.blink_time_ms + 100) * 1000);
    
    debug::info("Test complete, disconnecting from device");
    comms::USB_CDC.disconnect(connected_device);
    
    return 0;
}