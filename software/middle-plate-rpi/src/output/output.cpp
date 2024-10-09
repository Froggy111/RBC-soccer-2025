#include <iostream>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include <signal.h>

void HandleSignalOutput(int signal) {
	spdlog::info("[OUTPUT] Signal received: {0}", signal);
}

void OutputLoop() {
	spdlog::info("Ouput Loop started!");
	int output_id = getpid(), strategy_id = output_id - 1, input_id = output_id - 2;
	signal(SIGUSR1, HandleSignalOutput);
	while (true){
		sleep(2);
	}
}