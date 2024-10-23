#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/state_input.hpp"
#include "utils/fps.hpp"
#include "types/loop_types.hpp"

void InputLoop(InputStateManager& input_state_manager, FPS& fps_manager) {
	spdlog::info("Input Loop started!");
	while (true) {
		fps_manager.update_fps(INPUT);
		usleep(10000);
	}
}