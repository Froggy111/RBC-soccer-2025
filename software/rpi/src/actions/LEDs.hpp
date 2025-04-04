#pragma once
#include "comms.hpp"
#include "comms/identifiers.hpp"
#include "types.hpp"

namespace LEDs {

struct LEDBLinkerData {
	types::u8 id = 0;
	types::u8 RED = 0;
	types::u8 GREEN = 0;
	types::u8 BLUE = 0;
};

static void set_LED(const LEDBLinkerData &data) {
    comms::USB_CDC.writeToMiddlePico(comms::SendMiddlePicoIdentifiers::LEDs, (uint8_t *)&data, sizeof(data));
}

}