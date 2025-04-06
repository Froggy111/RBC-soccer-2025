#include "IMU.hpp"
#include "actions/kicker.hpp"
#include "attack.hpp"
#include "camera.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "mode_controller.hpp"
#include "motion.hpp"
#include "motors.hpp"
#include "processor.hpp"
#include "sensors/IR.hpp"
#include "sensors/line_sensors.hpp"
#include "types.hpp"
#include "wiringPi.h"
#include <cmath>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <thread>
#include <unistd.h>

camera::Camera cam;
camera::CamProcessor processor;
MotionController motion_controller;
line_sensors::LineSensors line_sensor;

bool start() {
    // // ^ Camera
    // if (!cam.initialize(camera::RES_480P)) {
    //     debug::error("INITIALIZED CAMERA - FAILED");
    //     return false;
    // } else {
    //     debug::info("INITIALIZED CAMERA - SUCCESS");
    // }
    //
    // if (!cam.startCapture(processor.process_frame)) {
    //     debug::error("INITIALIZED CAMERA CAPTURE - FAILED");
    //     return false;
    // } else {
    //     debug::info("INITIALIZED CAMERA CAPTURE - SUCCESS");
    // }

    // ^ Motion Control
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

    line_sensor.init();

    // // ^ IMU
    // IMU::init();
    //
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
    // cam.stopCapture();
}

int main() {
    // // * wiring PI setup
    // wiringPiSetupGpio();

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
        // * IR processing
        int max_IR_idx = 0, max_IR = 0;
        for (int i = 0; i < IR::SENSOR_COUNT; i++) {
            if (IR::IR_sensors.get_data_for_sensor_id(i) > max_IR) {
                max_IR_idx = i;
                max_IR     = IR::IR_sensors.get_data_for_sensor_id(i);
            }
        }

        float angle = max_IR_idx * (15.0f / 180.0f * M_PI) - M_PI * 3 / 2;
        if (angle < 0) {
            angle += M_PI * 2;
        }
        angle = M_PI * 2 - angle;
        if (angle > M_PI * 2) {
            angle = angle - M_PI * 2;
        }
        debug::info("IR angle: %f", angle);
        // processor.ball_heading = angle;

        // // * Attack strategy
        // types::Vec2f32 ball_pos(processor.ball_position.position.x,
        //                         processor.ball_position.position.y);
        // strategy::attack(ball_pos, M_PI - processor.goalpost_info.first.angle,
        //                  true, types::Vec2f32(0, 0));

        // DEFEND STRAT
        types::f32 angle_around_0 =
            (angle < M_PI) ? angle : -(M_PI * 2 - angle);
        types::f32 defend_move_intensity =
            std::remainder(std::fabs(angle_around_0), M_PI) / M_PI;
        defend_move_intensity *= 4;
        if (defend_move_intensity > 1) {
            defend_move_intensity = 1;
        }
        types::f32 defend_move_direction =
            (angle_around_0 >= 0) ? M_PI_2 : -M_PI_2;
        if (defend_move_intensity < 0.1) {
            defend_move_intensity = 0.1;
        }
        debug::info("defend move intensity: %f", defend_move_intensity);
        // Combine movement command with line avoidance
        types::Vec2f32 moveCommand =
            types::Vec2f32(defend_move_direction, defend_move_intensity);

        // Add the vectors in Cartesian space
        float moveX  = std::cos(defend_move_direction) * defend_move_intensity;
        float moveY  = std::sin(defend_move_direction) * defend_move_intensity;
        float finalX = moveX + line_sensor.evade_vector().x * defend_move_intensity;
        float finalY = moveY + line_sensor.evade_vector().y * defend_move_intensity;

        // Convert back to (angle, magnitude)
        float finalDirection = std::atan2(finalY, finalX);
        float finalIntensity = std::sqrt(finalX * finalX + finalY * finalY);
        // Clamp intensity if needed
        if (finalIntensity > 1.0)
            finalIntensity = 1.0;

        // Pass to motors::translate
        motors::translate(types::Vec2f32(finalDirection, finalIntensity));

        debug::info("line sensor evade vector: %f, %f",
                    line_sensor.evade_vector().x, line_sensor.evade_vector().y);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    stop();
    debug::info("EMERGENCY STOP DONE.");
    return 0;
}
