#include "src/main/loop.hpp"
#include "src/strategy/main.hpp"
#include "src/input/main.hpp"
#include "src/output/main.hpp"

void start_loops() {
	std::future<void> input_promise = std::async(InputLoop);
	std::future<void> strategy_promise = std::async(StrategyLoop);
	std::future<void> output_promise = std::async(OutputLoop);

	LoopPromises promises = {
		&input_promise,
		&strategy_promise,
		&output_promise
	};

	promises.input->wait();
	promises.strategy->get();
	promises.output->get();
}

int main() {
	start_loops();
}