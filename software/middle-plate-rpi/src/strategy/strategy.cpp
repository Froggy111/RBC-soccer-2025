#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/state_input.hpp"
#include "main/state_output.hpp"

void StrategyLoop(InputStateManager input_state_manager, OutputStateManager output_state_manager) {
	spdlog::info("Strategy Loop started!");
	while (true) {
		input_state_manager.update_state();
		usleep(10000);
		spdlog::info("Strategy Loop: Counter = {}", input_state_manager.read_state().counter);
	}
}