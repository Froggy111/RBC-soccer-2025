#include "IMU.hpp"
#include "actions/kicker.hpp"
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
#include <thread>
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

    motors::command_motor(1, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(2, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(3, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(4, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    debug::info("INITIALIZED MOTION CONTROL - SUCCESS");

    // ^ IR Sensors
    IR::IR_sensors.init();
    debug::info("INITIALIZED IR SENSORS - SUCCESS");

    // ^ IMU
    IMU::init();

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

    motion_controller.init(0.3f, 0.00f, 0.0f, 0.2f, 0.1f, 0.0f, 1.0f);

    // Main loop with emergency stop check
    while (mode_controller::mode != mode_controller::Mode::EMERGENCY_STOP) {
        int max_IR_idx = 0, max_IR = 0;
        for (int i = 0; i < IR::SENSOR_COUNT; i++) {
            if (IR::IR_sensors.get_data_for_sensor_id(i) > max_IR) {
                max_IR_idx = i;
                max_IR     = IR::IR_sensors.get_data_for_sensor_id(i);
            }
        }

        debug::info("IR Sensor %d: %d", max_IR_idx, max_IR);

        float angle = max_IR_idx * (15.0f / 180.0f * M_PI) - M_PI * 3 / 2;
        if (angle < 0) {
            angle += M_PI * 2;
        }
        angle = M_PI * 2 - angle;

        auto commands  = motion_controller.velocity_pid(0, angle, angle, 0.0f);
        auto commands2 = motion_controller.move_heading(angle, angle, 0.1f);

        motors::command_motor_motion_controller(
            1, (std::get<0>(commands) + std::get<0>(commands2)) * 4000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(
            2, (std::get<1>(commands) + std::get<1>(commands2)) * 4000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(
            3, (std::get<2>(commands) + std::get<2>(commands2)) * 4000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        motors::command_motor_motion_controller(
            4, (std::get<3>(commands) + std::get<3>(commands2)) * 4000);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    stop();
    debug::info("EMERGENCY STOP DONE.");
    return 0;
}
