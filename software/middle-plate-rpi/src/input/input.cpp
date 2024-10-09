#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"

void InputLoop() {
	spdlog::info("Input Loop started!");
	int input_id = getpid(), strategy_id = input_id + 1, output_id = input_id + 2;
}