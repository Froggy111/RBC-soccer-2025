#include "loop.hpp"
#include "strategy/main.hpp"
#include "input/main.hpp"
#include "output/main.hpp"
#include "spdlog/spdlog.h"
#include "main/input_state.hpp"

// Start the input, strategy, and output loops
void start_loops() {
	spdlog::info("Creating Loops");
	std::future<void> input_promise = std::async(InputLoop);
	std::future<void> strategy_promise = std::async(StrategyLoop);
	std::future<void> output_promise = std::async(OutputLoop);

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


	start_loops();
}