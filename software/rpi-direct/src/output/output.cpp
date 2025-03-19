#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/state_output.hpp"
#include "utils/fps.hpp"

void OutputLoop(OutputStateManager& output_state_manager, FPS& fps_manager) {
	spdlog::info("Ouput Loop started!");
}