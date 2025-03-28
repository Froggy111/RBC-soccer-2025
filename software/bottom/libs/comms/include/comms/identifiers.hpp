#pragma once
#include "types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 {
  COMMS_WARN = 0,  // warnings should be sent here
  COMMS_ERROR = 1, // hard errors sent here
  COMMS_DEBUG = 2, // everything should fall under here by default
  SPI_INIT_FAIL = 3,
  LINE_SENSOR_DATA = 4
};

enum class RecvIdentifiers : types::u8 {
  MOTOR_DRIVER_CMD = 0,
  KICKER_CMD = 1,
  DEBUG_TEST_BLINK = 255,
};

static const types::u16 identifier_arr_len = 256;

} // namespace comms
