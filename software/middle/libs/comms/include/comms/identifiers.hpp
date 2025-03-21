#pragma once
#include "types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 {
  COMMS_WARN,
  COMMS_ERROR,
  COMMS_DEBUG,
  TRACE,
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL,
};

enum class RecvIdentifiers : types::u8 {
  LEDs = 0,
  DEBUG_TEST_BLINK = 255,
};

static const types::u16 identifier_arr_len = 256;

} // namespace comms
