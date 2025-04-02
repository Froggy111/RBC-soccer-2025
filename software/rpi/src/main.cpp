#include "camera.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "mode_controller.hpp"
#include "motion.hpp"
#include "pinmap.hpp"
#include "processor.hpp"
#include "sensors/IR.hpp"
#include "wiringPi.h"
#include <cstdio>
#include <opencv2/opencv.hpp>

camera::Camera cam;
camera::CamProcessor processor;
MotionController motion_controller;

bool start() {
    // Initialize with desired resolution
    if (!cam.initialize(camera::RES_480P)) {
        debug::error("INITIALIZED CAMERA - FAILED");
        return false;
    } else {
        debug::info("INITIALIZED CAMERA - SUCCESS");
    }

    // Start capturing with our frame processor
    if (!cam.startCapture(processor.process_frame)) {
        debug::error("INITIALIZED CAMERA CAPTURE - FAILED");
        return false;
    } else {
        debug::info("INITIALIZED CAMERA CAPTURE - SUCCESS");
    }

    // motion_controller.startControlThread();
    // debug::info("INITIALIZED MOTION CONTROL - SUCCESS\n");

    IR::IR_sensors.init();
    debug::info("INITIALIZED IR SENSORS - SUCCESS");

    return true;
}

void stop() {
    debug::warn("STOPPING MOTION...\n");
    motion_controller.stopControlThread();

    for (int i = 1; i <= 4; i++) {
    }
    

    debug::warn("STOPPING CAMERA...");
    cam.stopCapture();

}

int main() {
    // * wiring PI setup
    wiringPiSetupGpio();

    // * Mode controller setup
    mode_controller::init_mode_controller();

    comms::USB_CDC.init();

    // Main loop with emergency stop check
    while (mode_controller::mode != mode_controller::Mode::EMERGENCY_STOP) {
        
    }

    stop();
    debug::info("EMERGENCY STOP DONE.");
    return 0;
}

// 1 2 3