#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include <signal.h>

void HandleSignalInput(int signal) {
	spdlog::info("[INPUT] Signal received: {0}", signal);
}

void InputLoop() {
	spdlog::info("Input Loop started!");
	int input_id = getpid(), strategy_id = input_id + 1, output_id = input_id + 2;

	while (true)
	{
		kill(output_id, SIGUSR1);
		spdlog::warn("Signal sent to Output {0}", output_id);
		sleep(1);
	}
}