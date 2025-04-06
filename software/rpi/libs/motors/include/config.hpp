#pragma once

namespace motors {

const bool DIRECTIONS[]              = {true, true, false, false};
const int MOTION_CONTROL_MOTOR_MAP[] = {1, 4, 2, 3};
const int MOTOR_MAX_DUTY_CYCLE       = 2000;

const int MOTOR_MIN_RAMP_TIME = 1000; // ms
const int MOTOR_MAX_DUTY_CYCLE_PER_MS =
    MOTOR_MAX_DUTY_CYCLE / MOTOR_MIN_RAMP_TIME;

} // namespace motors
