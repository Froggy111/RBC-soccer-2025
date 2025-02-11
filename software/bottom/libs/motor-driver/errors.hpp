#include "pins.hpp"
#include "spdlog/spdlog.h"
#include "registers.hpp"

class ErrorManager {
public:
	//! on error
	void handle_error(PinInputControl* inputControl,
		PinOutputControl* outputControl,
		Registers* registers);
};