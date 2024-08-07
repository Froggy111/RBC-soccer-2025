#include <iostream>
#include <unistd.h>

void OutputLoop() {
	for (int i = 0; i < 1000; i++){
		sleep(1);
		std::printf("OutputLoop: %d\n", i);
	}
}