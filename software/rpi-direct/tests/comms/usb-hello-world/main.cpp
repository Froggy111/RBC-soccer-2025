#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "debug.hpp"
#include <csignal>
#include <cstring>
#include <iostream>

// Flag for program termination
volatile bool running = true;

// Signal handler for Ctrl+C
void signalHandler(int signum) {
    std::cout << "Interrupt received, terminating..." << std::endl;
    running = false;
}

// Callback for handling raw data received from the Pico
void rawDataCallback(const types::u8 *data,
                     types::u16 length) {
    // Create a null-terminated string (assuming the data is text)
    char *text = new char[length + 1];
    std::memcpy(text, data, length);
    text[length] = '\0';

    std::cout << "Received: " << text << std::endl;
    delete[] text;
}

int main() {
    // Register signal handler for clean termination
    signal(SIGINT, signalHandler);

    debug::info("Starting USB listener for Pico...");

    // Initialize the communication library
    if (!comms::comms_init()) {
        debug::error("Failed to initialize communications");
        return 1;
    }

    debug::info("Communication initialized");
    debug::info("Scanning for Pico devices...");

    // Scan for connected devices
    auto devices = comms::get_connected_devices();

    if (devices.empty()) {
        debug::error(
            "No Pico devices found. Make sure your Pico is connected via USB.");
        return 1;
    }

    debug::info("Found %d USB devices", devices.size());

    // Connect to the first device or you can choose a specific one based on type
    usb::USBDevice connectedDevice;
    bool connected = false;

    for (const auto &device : devices) {
        debug::info("Found device: %s (%s) VID:PID %04x:%04x",
                    device.product.c_str(), device.serialNumber.c_str(),
                    device.vid, device.pid);

        // Try to connect
        usb::USBDevice tempDevice = device;
        if (comms::USB_CDC.connect(tempDevice)) {
            connectedDevice = tempDevice;
            connected       = true;
            debug::info("Connected to %s (%s)", connectedDevice.product.c_str(),
                        connectedDevice.serialNumber.c_str());
            break;
        }
    }

    if (!connected) {
        debug::error("Failed to connect to any Pico device");
        return 1;
    }

    // Register callback for debug messages (ID 2)
    comms::USB_CDC.register_callback(connectedDevice,
        (types::u8)comms::RecvBottomPicoIdentifiers::COMMS_DEBUG,
        rawDataCallback);

    debug::info("Listening for messages. Press Ctrl+C to exit...");

    // Main loop
    while (running) {
        // The USB CDC class handles reading in a separate thread, so we just need to sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    comms::USB_CDC.disconnect(connectedDevice);
    debug::info("Disconnected. Exiting...");

    return 0;
}