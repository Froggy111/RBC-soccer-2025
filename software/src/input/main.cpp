#include <iostream>
#include <unistd.h>

void InputLoop() {
	for (int i = 0; i < 1000; i++){
		sleep(1);
		std::printf("InputLoop: %d\n", i);
	}
}