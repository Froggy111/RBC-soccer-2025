#include "loop.hpp"
#include "strategy/main.hpp"
#include "input/main.hpp"
#include "output/main.hpp"
#include "spdlog/spdlog.h"
#include "main/input_state.hpp"
#include "main/output_state.hpp"

// Start the input, strategy, and output loops
void start_loops(InputStateManager input_state_manager, OutputStateManager output_state_manager) {
	spdlog::info("Creating Loops");
    std::future<void> input_promise = std::async([&]() { InputLoop(input_state_manager); });
    std::future<void> strategy_promise = std::async([&]() { StrategyLoop(input_state_manager, output_state_manager); });
    std::future<void> output_promise = std::async([&]() { OutputLoop(output_state_manager); });

	LoopPromises promises = {
		&input_promise,
		&strategy_promise,
		&output_promise
	};

	spdlog::info("Loops successfully started!");
	promises.input->wait();
	promises.strategy->get();
	promises.output->get();
}

int main() {
	std::printf("\n\
 _____  ____   _____   _ __          _________   ___   ___ ___  _____ \n\
|  __ \\|  _ \\ / ____| | |\\ \\        / /__   __| |__ \\ / _ \\__ \\| ____|\n\
| |__) | |_) | |      | | \\ \\  /\\  / /   | |       ) | | | | ) | |__  \n\
|  _  /|  _ <| |      | |  \\ \\/  \\/ /    | |      / /| | | |/ /|___ \\ \n\
| | \\ \\| |_) | |____  | |___\\  /\\  /     | |     / /_| |_| / /_ ___) |\n\
|_|  \\_\\____/ \\_____| |______\\/  \\/      |_|    |____|\\___/____|____/ \n\
");

	// create state variables
	InputStateManager input_state_manager = InputStateManager();
	OutputStateManager output_state_manager = OutputStateManager();

	start_loops(input_state_manager, output_state_manager);
}