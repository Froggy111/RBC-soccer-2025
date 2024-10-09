#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"

void HandleSignalStrategy(int signal) {
	spdlog::info("[STRATEGY] Signal received: {0}", signal);
}

void StrategyLoop() {
	spdlog::info("Strategy Loop started!");
	int strategy_id = getpid(), input_id = strategy_id - 1, output_id = strategy_id + 1;
}