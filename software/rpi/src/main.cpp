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

int main() {
    motors::command_motor(1, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(2, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(3, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    motors::command_motor(4, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    debug::info("EMERGENCY STOP DONE.");
    return 0;
}
