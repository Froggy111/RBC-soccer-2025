#include "IR.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>

// Flag for program termination
volatile bool running = true;

// Signal handler for Ctrl+C
void signalHandler(int signum) {
    std::cout << "Interrupt received, terminating..." << std::endl;
    running = false;
}

int main() {
    // Register signal handler for clean termination
    signal(SIGINT, signalHandler);

    debug::info("Starting USB communication with top plate...");

    // Initialize the communication library
    if (!comms::USB_CDC.init()) {
        debug::error("Failed to initialize communications");
        return 1;
    }

    debug::info("Communication initialized");

    debug::info(
        "Listening for messages from top plate. Press Ctrl+C to exit...");
    debug::info("Will send test commands when device is connected...");

    while (!comms::USB_CDC.is_connected(usb::DeviceType::TOP_PLATE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    IR::IR_sensors.init();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    debug::info("Exiting...");
    return 0;
}
