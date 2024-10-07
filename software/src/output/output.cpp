#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "main/output_state.hpp"

void OutputLoop(OutputStateManager output_state_manager) {
	spdlog::info("Ouput Loop started!");
}