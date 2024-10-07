#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/input_state.hpp"

void InputLoop() {
	spdlog::info("Input Loop started!");
	while (true) {
		input_state_manager.update_state();
		usleep(10000);
		spdlog::info("Input Loop: Counter = {}", input_state_manager.read_state().counter);
	}
}