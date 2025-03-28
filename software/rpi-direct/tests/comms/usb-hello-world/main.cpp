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

// Callback for handling debug messages from the bottom plate
void debugCallback(const types::u8 *data, types::u16 length) {
    // Create a null-terminated string (assuming the data is text)
    char *text = new char[length + 1];
    std::memcpy(text, data, length);
    text[length] = '\0';

    std::cout << "Received from bottom plate: " << text << std::endl;
    delete[] text;
}

int main() {
    // Register signal handler for clean termination
    signal(SIGINT, signalHandler);

    debug::info("Starting USB listener for bottom plate...");

    // Initialize the communication library
    if (!comms::USB_CDC.init()) {
        debug::error("Failed to initialize communications");
        return 1;
    }

    debug::info("Communication initialized");

    // Register callback for the debug message type from bottom plate
    comms::USB_CDC.register_callback(usb::DeviceType::TOP_PLATE, 
        (types::u8)comms::RecvBottomPicoIdentifiers::COMMS_DEBUG, 
        debugCallback);

    debug::info("Listening for messages from bottom plate. Press Ctrl+C to exit...");

    // Main loop
    while (running) {
        // The comms library handles device scanning and message reading in the background
        
        // Periodically check if bottom plate is connected
        static int counter = 0;
        if (counter++ % 20 == 0) {  // Check approximately every 2 seconds
            if (comms::USB_CDC.is_connected(usb::DeviceType::BOTTOM_PLATE)) {
                debug::info("Bottom plate is connected");
            } else {
                debug::info("Waiting for bottom plate to connect...");
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    debug::info("Exiting...");
    return 0;
}