#pragma once

#include <string>

#include "identifiers.hpp"
#include "types.hpp"
#include "comms.hpp"

namespace debug {

enum class LogLevel : types::u8 {
  TRACE = (types::u8)comms::SendIdentifiers::TRACE,
  DEBUG = (types::u8)comms::SendIdentifiers::DEBUG,
  INFO = (types::u8)comms::SendIdentifiers::INFO,
  WARN = (types::u8)comms::SendIdentifiers::WARN,
  ERROR = (types::u8)comms::SendIdentifiers::ERROR,
  FATAL = (types::u8)comms::SendIdentifiers::FATAL,
};

void msg(LogLevel log_level, std::string format, ...);
void msg_UART(LogLevel log_level, std::string format, ...);

} // namespace debug
