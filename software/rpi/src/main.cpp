#include "camera.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "mode_controller.hpp"
#include "motion.hpp"
#include "motors.hpp"
#include "processor.hpp"
#include "sensors/IR.hpp"
#include "wiringPi.h"
#include <cmath>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <unistd.h>

camera::Camera cam;
camera::CamProcessor processor;
MotionController motion_controller;

bool start() {
    // ^ Camera
    if (!cam.initialize(camera::RES_480P)) {
        debug::error("INITIALIZED CAMERA - FAILED");
        return false;
    } else {
        debug::info("INITIALIZED CAMERA - SUCCESS");
    }

    if (!cam.startCapture(processor.process_frame)) {
        debug::error("INITIALIZED CAMERA CAPTURE - FAILED");
        return false;
    } else {
        debug::info("INITIALIZED CAMERA CAPTURE - SUCCESS");
    }

    // ^ Motion
    // Reset motors
    motors::command_motor(1, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(2, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(3, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(4, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    // motion_controller.startControlThread();
    debug::info("INITIALIZED MOTION CONTROL - SUCCESS");

    // ^ IR Sensors
    IR::IR_sensors.init();
    debug::info("INITIALIZED IR SENSORS - SUCCESS");

    return true;
}

void stop() {
    // set a timeout for _exit in 2 seconds
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        _exit(0);
    }).detach();

    // ^ Stop Motion
    debug::warn("STOPPING MOTION...\n");
    // motion_controller.stopControlThread();

    motors::command_motor(1, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(2, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(3, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(4, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    // ^ Stop Camera
    debug::warn("STOPPING CAMERA...");
    cam.stopCapture();
}

int main() {
    // * wiring PI setup
    wiringPiSetupGpio();

    // * Mode controller setup
    mode_controller::init_mode_controller();

    comms::USB_CDC.init();

    if (!start()) {
        debug::error("INITIALIZATION - FAILED");
        return 1;
    }

    // Main loop with emergency stop check
    while (mode_controller::mode != mode_controller::Mode::EMERGENCY_STOP) {
        auto commands = motion_controller.velocity_pid(
            processor.current_pos.heading, M_PI, M_PI, 0.1f);

        motors::command_motor_motion_controller(1,
                                                std::get<0>(commands) * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(2,
                                                std::get<1>(commands) * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(3,
                                                std::get<2>(commands) * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(4,
                                                std::get<3>(commands) * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        motors::command_motor_motion_controller(1, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(2, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(3, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(4, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    stop();
    debug::info("EMERGENCY STOP DONE.");
    return 0;
}