#pragma once
#include "types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 {
  COMMS_WARN,
  COMMS_ERROR,
  TRACE,
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL,
};

enum class RecvIdentifiers : types::u8 {

};

static const types::u16 identifier_arr_len = 256;

} // namespace comms
