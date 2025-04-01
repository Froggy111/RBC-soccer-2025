#include "camera.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "mode_controller.hpp"
#include "motion.hpp"
#include "processor.hpp"
#include "wiringPi.h"
#include <cstdio>
#include <opencv2/opencv.hpp>

camera::Camera cam;
camera::CamProcessor processor;
MotionController motion_controller;

bool start() {
    // Initialize with desired resolution
    if (!cam.initialize(camera::RES_480P)) {
        debug::error("Failed to initialize camera\n");
        return false;
    } else {
        debug::info("Camera Capture Initialized");
    }

    // Start capturing with our frame processor
    if (!cam.startCapture(processor.process_frame)) {
        debug::error("Failed to start camera capture\n");
        return false;
    } else {
        debug::info("Camera capture started successfully\n");
    }

    motion_controller.startControlThread();

    return true;
}

void stop() {
    debug::info("Stopping motion controller...\n");
    motion_controller.stopControlThread();

    debug::info("Stopping camera capture...\n");
    cam.stopCapture();

    debug::info("Stopping motion control thread...\n");
    motion_controller.stopControlThread();
}

int main() {
    comms::init();

    // * wiring PI setup
    wiringPiSetupGpio();

    // * Mode controller setup
    mode_controller::init_mode_controller();

    // Main loop with emergency stop check
    while (mode_controller::mode != mode_controller::Mode::EMERGENCY_STOP) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    stop();
    debug::info("EMERGENCY STOP DONE.\n");
    return 0;
}