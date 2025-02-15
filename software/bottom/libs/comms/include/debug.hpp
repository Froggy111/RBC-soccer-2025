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

void msg(std::string format, LogLevel log_level=LogLevel::INFO ...);
void msg_UART(std::string format, LogLevel log_level=LogLevel::INFO ...);

} // namespace debug
