#include "loop.hpp"
#include "strategy/strategy.hpp"
#include "input/input.hpp"
#include "output/output.hpp"
#include "spdlog/spdlog.h"
#include "main/state_input.hpp"
#include "main/state_output.hpp"

// Start the input, strategy, and output loops
// void start_loops(InputStateManager input_state_manager, OutputStateManager output_state_manager) {
// 	spdlog::info("Creating Loops");
//     std::future<void> input_promise = std::async([&]() { InputLoop(input_state_manager); });
//     std::future<void> strategy_promise = std::async([&]() { StrategyLoop(input_state_manager, output_state_manager); });
//     std::future<void> output_promise = std::async([&]() { OutputLoop(output_state_manager); });

// 	LoopPromises promises = {
// 		&input_promise,
// 		&strategy_promise,
// 		&output_promise
// 	};

// 	spdlog::info("Loops successfully started!");
// 	promises.input->wait();
// 	promises.strategy->get();
// 	promises.output->get();
// }

int main()
{
	// create processes
	pid_t slave = fork();

	// create input, output, and strategy processes IF ITS NOT THE MASTER
	pid_t input_slave = slave == 0 ? 0 : fork()
	pid_t output_slave = slave == 0 ? 0 : fork()
	pid_t strategy_slave = slave == 0 ? 0 : fork();

	switch (i + 1)
	{
	case 0:
		std::printf("\n\
 _____  ____   _____   _ __          _________   ___   ___ ___  _____ \n\
|  __ \\|  _ \\ / ____| | |\\ \\        / /__   __| |__ \\ / _ \\__ \\| ____|\n\
| |__) | |_) | |      | | \\ \\  /\\  / /   | |       ) | | | | ) | |__  \n\
|  _  /|  _ <| |      | |  \\ \\/  \\/ /    | |      / /| | | |/ /|___ \\ \n\
| | \\ \\| |_) | |____  | |___\\  /\\  /     | |     / /_| |_| / /_ ___) |\n\
|_|  \\_\\____/ \\_____| |______\\/  \\/      |_|    |____|\\___/____|____/ \n\
");
        break;
    case 1:
        InputLoop()
		break;
	case 2:

		break;
	case 3:

        break;
    default:
        spdlog::error("Something went wrong during while forking...");
	}


	// create state variables
	// InputStateManager input_state_manager = InputStateManager();
	// OutputStateManager output_state_manager = OutputStateManager();
}