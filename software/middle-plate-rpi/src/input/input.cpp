#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/state_input.hpp"

void InputLoop(InputStateManager& input_state_manager) {
	spdlog::info("Input Loop started!");
	while (true) {
		usleep(10000);
	}
}