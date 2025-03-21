#pragma once

#include "comms/identifiers.hpp"
#include "types.hpp"
#include "comms.hpp"

namespace debug {

enum class LogLevel : types::u8 {
  TRACE,
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL,
};

void log(char *format, ...);
void debug(char *format, ...);
void info(char *format, ...);
void warn(char *format, ...);
void error(char *format, ...);
void fatal(char *format, ...);

void msg(std::string format, LogLevel log_level = LogLevel::INFO, ...);
void msg_UART(std::string format, LogLevel log_level = LogLevel::INFO, ...);

} // namespace debug
