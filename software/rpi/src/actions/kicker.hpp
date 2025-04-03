#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "comms/usb.hpp"

namespace kicker {

const struct KickerData {
	uint16_t pulse_duration = 100;
} kicker_data;

static void send_kick(void) {
	comms::USB_CDC.writeToBottomPico(comms::SendBottomPicoIdentifiers::KICKER_CMD, 
		(uint8_t *)&kicker_data, sizeof(kicker_data));
}

}