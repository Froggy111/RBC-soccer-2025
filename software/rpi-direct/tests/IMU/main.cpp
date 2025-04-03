#include "IMU.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include <iostream>
#include <thread>

extern "C" {

#include <csignal>
#include <cstring>
}

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

    IMU::init();

    while (true) {
        std::cout << "Integrated IMU data:\nposition: " << IMU::position()
                  << "\nvelocity: " << IMU::velocity()
                  << "\naccel: " << IMU::accel()
                  << "\norientation: " << IMU::orientation()
                  << "\nangular velocity: " << IMU::angular_velocity()
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    debug::info("Exiting...");
    return 0;
}
