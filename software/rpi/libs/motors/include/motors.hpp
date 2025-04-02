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
bool command_motor(int id, int duty_cycle);

}