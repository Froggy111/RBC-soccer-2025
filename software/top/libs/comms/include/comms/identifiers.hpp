#pragma once
#include "types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 {
  DEBUG = 0,
  ICM29048 = 1,
};

enum class RecvIdentifiers : types::u8 {
  LEDs = 1
};

static const types::u16 identifier_arr_len = 256;

} // namespace comms
