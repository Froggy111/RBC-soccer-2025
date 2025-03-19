#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/state_input.hpp"
#include "main/state_output.hpp"
#include "utils/fps.hpp"
#include "types/loop_types.hpp"

void StrategyLoop(InputStateManager& input_state_manager, OutputStateManager& output_state_manager, FPS& fps_manager) {
	spdlog::info("Strategy Loop started!");

	while (true) {
		fps_manager.update_fps(STRATEGY);
		usleep(10000);
	}
}