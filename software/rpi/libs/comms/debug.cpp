#include <cstdarg>
#include <cstdio>
#include "debug.hpp"

namespace debug {

void log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("[LOG - RPI] %s\n", buffer);
    va_end(args);
}

void debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("[DEBUG - RPI] %s\n", buffer);
    va_end(args);
}

void info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("[INFO - RPI] %s\n", buffer);
    va_end(args);
}

void warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("[WARN - RPI] %s\n", buffer);
    va_end(args);
}

void error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    fprintf(stderr, "[ERROR - RPI] %s\n", buffer);
    va_end(args);
}

void fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    fprintf(stderr, "[FATAL - RPI] %s\n", buffer);
    va_end(args);
}
} // namespace debug