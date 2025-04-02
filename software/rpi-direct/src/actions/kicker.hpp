#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "comms/usb.hpp"

namespace kicker {

struct KickerData {
	uint16_t pulse_duration = 100;
} kicker_data;

void send_kick(void) {
	comms::USB_CDC.write(usb::DeviceType::BOTTOM_PLATE, (types::u8) comms::SendBottomPicoIdentifiers::KICKER_CMD, 
		(uint8_t *)&kicker_data, sizeof(kicker_data));
}

}