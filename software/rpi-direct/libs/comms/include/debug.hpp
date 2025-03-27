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

void log(const char *format, ...);
void debug(const char *format, ...);
void info(const char *format, ...);
void warn(const char *format, ...);
void error(const char *format, ...);
void fatal(const char *format, ...);

} // namespace debug