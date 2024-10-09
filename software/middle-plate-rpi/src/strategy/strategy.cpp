#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/state_input.hpp"
#include "main/state_output.hpp"

void StrategyLoop(InputStateManager input_state_manager, OutputStateManager output_state_manager) {
	spdlog::info("Strategy Loop started!");
	input_state_manager.add_function([&](InputState state) {
		spdlog::info("Strategy Loop Output: Counter = {}", state.counter);
	});

	while (true) {
		usleep(10000);
	}
}