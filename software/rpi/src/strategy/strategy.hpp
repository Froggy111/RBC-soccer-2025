#pragma once
#include "main/state_input.hpp"
#include "main/state_output.hpp"
#include "utils/fps.hpp"

void StrategyLoop(InputStateManager& input_state_manager, OutputStateManager& output_state_manager, FPS& fps_manager);