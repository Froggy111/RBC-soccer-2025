#pragma once

namespace mode_controller {

enum Mode {
	RUNNING,
	EMERGENCY_STOP
};

extern Mode mode;
        
void signal_handler(int signum);

void change_mode(Mode Mode);

void init_mode_controller();

}