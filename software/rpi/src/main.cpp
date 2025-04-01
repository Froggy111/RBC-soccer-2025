#include "camera.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "mode_controller.hpp"
#include "motion.hpp"
#include "processor.hpp"
#include "wiringPi.h"
#include <cstdio>
#include <opencv2/opencv.hpp>
#include "IR.hpp"

camera::Camera cam;
camera::CamProcessor processor;
MotionController motion_controller;

bool start() {
    // Initialize with desired resolution
    if (!cam.initialize(camera::RES_480P)) {
        debug::error("INITIALIZED CAMERA - FAILED\n");
        return false;
    } else {
        debug::info("INITIALIZED CAMERA - SUCCESS\n");
    }

    // Start capturing with our frame processor
    if (!cam.startCapture(processor.process_frame)) {
        debug::error("INITIALIZED CAMERA CAPTURE - FAILED\n");
        return false;
    } else {
        debug::info("INITIALIZED CAMERA CAPTURE - SUCCESS\n");
    }

    // motion_controller.startControlThread();
    // debug::info("INITIALIZED MOTION CONTROL - SUCCESS\n");

    IR::IR_sensors.init();
    debug::info("INITIALIZED IR SENSORS - SUCCESS\n");

    return true;
}

void stop() {
    debug::info("Stopping motion controller...\n");
    motion_controller.stopControlThread();

    debug::info("Stopping camera capture...\n");
    cam.stopCapture();

    // debug::info("Stopping motion control thread...\n");
    // motion_controller.stopControlThread();
}

struct MotorRecvData {
    uint8_t id;
    uint16_t duty_cycle;
};

int main() {
    // * wiring PI setup
    wiringPiSetupGpio();

    // * Mode controller setup
    mode_controller::init_mode_controller();

    comms::USB_CDC.init();

    // Main loop with emergency stop check
    while (mode_controller::mode != mode_controller::Mode::EMERGENCY_STOP) {
        for (int i = 0; i < 4; i++) {
            MotorRecvData motor_data = {
                .id         = (uint8_t)i,
                .duty_cycle = (uint16_t)(1000)};

            comms::USB_CDC.write(
                usb::DeviceType::BOTTOM_PLATE,
                (types::u8)comms::SendBottomPicoIdentifiers::MOTOR_DRIVER_CMD,
                reinterpret_cast<uint8_t *>(&motor_data), sizeof(motor_data));
            debug::info("Motor %d duty cycle: %d\n", i, motor_data.duty_cycle);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    stop();
    debug::info("EMERGENCY STOP DONE.\n");
    return 0;
}