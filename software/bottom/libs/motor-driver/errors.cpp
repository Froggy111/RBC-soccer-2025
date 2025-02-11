#include "pins.hpp"
#include "spdlog/spdlog.h"
#include "registers.hpp"

class ErrorManager {
	//! on error
	void handle_error(PinInputControl* inputControl,
		PinOutputControl* outputControl,
		Registers* registers)
	{
		spdlog::error("---> DRV8244 Fault Detected!");

		bool isActive = inputControl->get_last_value(PinMap::NSLEEP);
		if (isActive)
		{
			spdlog::info("Driver is ACTIVE. Reading active state registers...");

			// Read registers that provide diagnostic data during active operation.
			// (Replace register addresses with the correct ones from your datasheet.)
			uint8_t faultSummary = registers->readRegister(0x01); // e.g., FAULT_SUMMARY register
			uint8_t status1 = registers->readRegister(0x02);      // e.g., STATUS1 register
			uint8_t status2 = registers->readRegister(0x03);      // e.g., STATUS2 register

			spdlog::info("FAULT_SUMMARY: 0x{:02X}", faultSummary);
			spdlog::info("STATUS1:       0x{:02X}", status1);
			spdlog::info("STATUS2:       0x{:02X}", status2);
		}
		else
		{
			spdlog::info("Driver is in STANDBY.");
			// TODO: Implement standby state fault handling
		}

		// * try to clear the fault
		spdlog::info("Attempting to clear the fault...");
		// TODO: Implement fault clearing
	}

};