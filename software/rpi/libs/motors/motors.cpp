#include "motors.hpp"
#include "comms.hpp"
#include "config.hpp"
#include "debug.hpp"

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

    debug::info("Commanding motor %u with duty cycle %d", id, duty_cycle);

    MotorRecvData motor_data = {.id = id, .duty_cycle = duty_cycle};

    if (!DIRECTIONS[id - 1]) {
        motor_data.duty_cycle = -motor_data.duty_cycle;
    }

    // debug::info("Motor %d duty cycle %d", id, motor_data.duty_cycle);

    return comms::USB_CDC.writeToBottomPico(
        comms::SendBottomPicoIdentifiers::MOTOR_DRIVER_CMD,
        reinterpret_cast<uint8_t *>(&motor_data), sizeof(motor_data));
}

bool command_motor_motion_controller(uint8_t id, types::i16 duty_cycle) {
    return command_motor(MOTION_CONTROL_MOTOR_MAP[id - 1], duty_cycle);
}
} // namespace motors
