#pragma once
#include "types.hpp"

namespace motors {

struct MotorRecvData {
    uint8_t id;
    types::i16 duty_cycle;
} __attribute__((packed));

/**
 * @brief Takes in a motor id and a duty cycle and sends the command to the motor driver
 * ^ Accounts for all the motor direction and mapping issues
 * 
 * @param id 
 * @param duty_cycle 
 * @return true 
 * @return false 
 */
bool command_motor(uint8_t id, types::i16 duty_cycle);

/**
 * @brief Takes in a motor id and a duty cycle and sends the command to the motor driver
 * ^ Accounts for all the motor direction and mapping issues
 * ^ AND IMPORTANTLY, it maps the motors to the ones used in the motion control
 * 
 * @param id 
 * @param duty_cycle 
 * @return true 
 * @return false 
 */
bool command_motor_motion_controller(uint8_t id, types::i16 duty_cycle);

} // namespace motors