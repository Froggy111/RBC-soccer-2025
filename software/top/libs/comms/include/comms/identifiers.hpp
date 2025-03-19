#pragma once
#include "types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 {
  COMMS_WARN = 0,
  COMMS_ERROR = 1,
  COMMS_DEBUG = 2,
  TRACE = 3,
  DEBUG = 4,
  INFO = 5,
  WARN = 6,
  ERROR = 7,
  FATAL = 8,
  ICM29048 = 1,
};

enum class RecvIdentifiers : types::u8 {
  LEDs = 1
};

static const types::u16 identifier_arr_len = 256;

} // namespace comms
