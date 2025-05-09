#include "mode_controller.hpp"
#include "debug.hpp"
#include "pinmap.hpp"
#include <csignal>
#include <signal.h>
#include <unistd.h>
extern "C" {
#include <wiringPi.h>
}

namespace mode_controller {

Mode mode = Mode::IDLE;

void signal_handler(int signum) {
    if (signum == SIGINT) {
        // ! Emergency Stop
        change_mode(Mode::EMERGENCY_STOP);
    }
}

void change_mode(Mode new_mode) {
    if (mode == new_mode)
        return;

    mode = new_mode;
    switch (mode) {
        case Mode::RUNNING:
            debug::info("=============== RUNNING ===============");
            // digitalWrite((int)pinmap::RPI::BOTTOMPICO_TOGGLE_RUN, 1);
            // digitalWrite((int)pinmap::RPI::MIDPICO_TOGGLE_RUN, 1);
            // digitalWrite((int)pinmap::RPI::TOPPICO_TOGGLE_RUN, 1);
            break;

        case Mode::EMERGENCY_STOP:
            debug::info("=============== EMERGENCY ===============");
            // digitalWrite((int) pinmap::RPI::BOTTOMPICO_TOGGLE_RUN, 0);
            // digitalWrite((int) pinmap::RPI::MIDPICO_TOGGLE_RUN, 0);
            // digitalWrite((int)pinmap::RPI::TOPPICO_TOGGLE_RUN, 0);
            break;

        default: break;
    }
}

void init_mode_controller() {
    signal(SIGINT, signal_handler);

    // * Initialize all RUN pins
    // pinMode((int)pinmap::RPI::BOTTOMPICO_TOGGLE_RUN, OUTPUT);
    // pinMode((int)pinmap::RPI::MIDPICO_TOGGLE_RUN, OUTPUT);
    // pinMode((int)pinmap::RPI::TOPPICO_TOGGLE_RUN, OUTPUT);

    change_mode(Mode::RUNNING);
}

} // namespace mode_controller