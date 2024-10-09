#include "strategy/strategy.hpp"
#include "input/input.hpp"
#include "output/output.hpp"
#include "spdlog/spdlog.h"
#include <signal.h>

int main()
{
    std::printf("\n\
_____  ____   _____   _ __          _________   ___   ___ ___  _____ \n\
|  __ \\|  _ \\ / ____| | |\\ \\        / /__   __| |__ \\ / _ \\__ \\| ____|\n\
| |__) | |_) | |      | | \\ \\  /\\  / /   | |       ) |  | | ) | |__  \n\
|  _  /|  _ <| |      | |  \\ \\/  \\/ /    | |      / /| | | |/ /|___ \\ \n\
| | \\ \\| |_) | |____  | |___\\  /\\  /     | |     / /_| |_| / /_ ___) |\n\
|_|  \\_\\____/ \\_____| |______\\/  \\/      |_|    |____|\\___/____|____/ \n\
");

    // create processes
    pid_t slave1 = fork();
    if (slave1 == 0) {
        InputLoop();
        exit(0);
    }

    pid_t slave2 = fork();
    if (slave2 == 0) {
        StrategyLoop();
        exit(0);
    }

    pid_t slave3 = fork();
    if (slave3 == 0) {
        OutputLoop();
        exit(0);
    }

    spdlog::info("Slaves with ID, {0}, {1}, {2}, have been created", slave1, slave2, slave3);
}