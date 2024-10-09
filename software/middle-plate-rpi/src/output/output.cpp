#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"

void OutputLoop() {
	spdlog::info("Ouput Loop started!");
	int output_id = getpid(), strategy_id = output_id - 1, input_id = output_id - 2;
}