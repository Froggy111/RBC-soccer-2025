#include "motors.hpp"
#include "comms.hpp"
#include "config.hpp"
#include "debug.hpp"
#include "motion.hpp"

using namespace types;

namespace motors {

bool command_motor(uint8_t id, types::i16 duty_cycle) {
    if (abs(duty_cycle) > MOTOR_MAX_DUTY_CYCLE) {
        debug::error("Motor duty cycle %d is too high, max is %d", duty_cycle,
                     MOTOR_MAX_DUTY_CYCLE);
        return false;
    }

    if (id < 1 || id >= 5) {
        debug::error("Motor ID %d is out of range", id);
        return false;
    }

    MotorRecvData motor_data = {.id = id, .duty_cycle = duty_cycle};

    if (!DIRECTIONS[id - 1]) {
        motor_data.duty_cycle = -motor_data.duty_cycle;
    }

    return comms::USB_CDC.writeToBottomPico(
        comms::SendBottomPicoIdentifiers::MOTOR_DRIVER_CMD,
        reinterpret_cast<uint8_t *>(&motor_data), sizeof(motor_data));
}

bool command_motor_motion_controller(uint8_t id, types::i16 duty_cycle) {
    return command_motor(MOTION_CONTROL_MOTOR_MAP[id - 1], duty_cycle);
}

void translate(types::Vec2f32 vec) {
    auto commands = motion_controller.move_heading(0.0f, 0.0f, 0.2f);
    motors::command_motor_motion_controller(1, std::get<0>(commands) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    motors::command_motor_motion_controller(2, std::get<1>(commands) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    motors::command_motor_motion_controller(3, std::get<2>(commands) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    motors::command_motor_motion_controller(4, std::get<3>(commands) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    debug::info("Motor commands: %d %d %d %d",
                std::get<0>(commands) * MOTOR_MAX_DUTY_CYCLE,
                std::get<1>(commands) * MOTOR_MAX_DUTY_CYCLE,
                std::get<2>(commands) * MOTOR_MAX_DUTY_CYCLE,
                std::get<3>(commands) *
                    MOTOR_MAX_DUTY_CYCLE); // 4.... (big number) 0 1 0
}

std::tuple<f32, f32, f32, f32> operator+(std::tuple<f32, f32, f32, f32> &a,
                                         std::tuple<f32, f32, f32, f32> &b) {
    return std::tuple<f32, f32, f32, f32>(
        std::get<0>(a) + std::get<0>(b), std::get<1>(a) + std::get<1>(b),
        std::get<2>(a) + std::get<2>(b), std::get<3>(a) + std::get<3>(b));
}

void translate_with_target_heading(f32 speed, f32 translate_heading,
                                   f32 orientation_heading) {
    auto translate_command =
        motion_controller.move_heading(0, -translate_heading, speed);
    auto orient_command = motion_controller.velocity_pid(
        0, -orientation_heading, -orientation_heading, 0);
    auto summed_command = translate_command + orient_command;
    motors::command_motor_motion_controller(1, std::get<0>(summed_command) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    motors::command_motor_motion_controller(2, std::get<1>(summed_command) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    motors::command_motor_motion_controller(3, std::get<2>(summed_command) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    motors::command_motor_motion_controller(4, std::get<3>(summed_command) *
                                                   MOTOR_MAX_DUTY_CYCLE);
    debug::info("Motor summed_command: %d %d %d %d",
                std::get<0>(summed_command) * MOTOR_MAX_DUTY_CYCLE,
                std::get<1>(summed_command) * MOTOR_MAX_DUTY_CYCLE,
                std::get<2>(summed_command) * MOTOR_MAX_DUTY_CYCLE,
                std::get<3>(summed_command) *
                    MOTOR_MAX_DUTY_CYCLE); // 4.... (big number) 0 1 0
}
} // namespace motors
