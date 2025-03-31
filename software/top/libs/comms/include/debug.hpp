#pragma once

#include "types.hpp"
namespace debug {

void log(const char *format, ...);
void debug(const char *format, ...);
void info(const char *format, ...);
void warn(const char *format, ...);
void error(const char *format, ...);
void fatal(const char *format, ...);

} // namespace debug
