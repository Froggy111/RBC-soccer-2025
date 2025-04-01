#pragma once
#include "types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 {
  COMMS_WARN = 0,  // warnings should be sent here
  COMMS_ERROR = 1, // hard errors sent here
  COMMS_DEBUG = 2, // everything should fall under here by default
  SPI_FAIL = 3,
  LED_LISTENER_FAIL = 4,
  ICM29048 = 5,
  PING = 254,
  BOARD_ID = 255,
};

enum class RecvIdentifiers : types::u8 {
  LEDs = 0,
  PING = 253,
  BOARD_ID = 254,
  BLINK = 255,
};

enum class BoardIdentifiers : types::u8 {
  BOTTOM_PICO = 0,
  MIDDLE_PICO = 1,
  TOP_PICO = 2,
};

static const types::u16 identifier_arr_len = 256;

} // namespace comms
