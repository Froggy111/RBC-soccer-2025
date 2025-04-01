#include <atomic>
#include "debug.hpp"
#include <signal.h>
#include "mode_controller.hpp"
#include "pinmap.hpp"
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
            debug::info("Set to RUNNING");
            digitalWrite((int)pinmap::PI::BOTTOMPICO_TOGGLE_RUN, 1);
            digitalWrite((int)pinmap::PI::MIDPICO_TOGGLE_RUN, 1);
            digitalWrite((int)pinmap::PI::TOPPICO_TOGGLE_RUN, 1);
            break;

        case Mode::EMERGENCY_STOP:
            debug::info("Emergency stop triggered (Ctrl+C)");
            digitalWrite((int) pinmap::PI::BOTTOMPICO_TOGGLE_RUN, 0);
            digitalWrite((int) pinmap::PI::MIDPICO_TOGGLE_RUN, 0);
            digitalWrite((int) pinmap::PI::TOPPICO_TOGGLE_RUN, 0);
            break;

        default:
            break;
    }
}

void init_mode_controller() {
    signal(SIGINT, signal_handler);

    // * Initialize all RUN pins
    pinMode((int)pinmap::PI::BOTTOMPICO_TOGGLE_RUN, OUTPUT);
    pinMode((int)pinmap::PI::MIDPICO_TOGGLE_RUN, OUTPUT);
    pinMode((int)pinmap::PI::TOPPICO_TOGGLE_RUN, OUTPUT);

    change_mode(Mode::RUNNING);
}

}