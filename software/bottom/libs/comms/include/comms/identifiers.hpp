#pragma once
#include "types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 {
  TRACE = 249,
  DEBUG = 250,
  INFO = 251,
  WARN = 252,
  ERROR = 253,
  FATAL = 254,
  COMMS_ERROR = 255
};

enum class RecvIdentifiers : types::u8 {

};

} // namespace comms
