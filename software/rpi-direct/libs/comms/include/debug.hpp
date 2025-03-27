#pragma once

#include "comms/identifiers.hpp"
#include "types.hpp"
#include "comms.hpp"
#include <string>

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

void msg(std::string format, LogLevel log_level = LogLevel::INFO, ...);
void msg_device(const usb::USBDevice& device, std::string format, LogLevel log_level = LogLevel::INFO, ...);

} // namespace debug