#pragma once

#include "types.hpp"
namespace debug {

void log(char *format, ...);
void debug(char *format, ...);
void info(char *format, ...);
void warn(char *format, ...);
void error(char *format, ...);
void fatal(char *format, ...);

} // namespace debug
