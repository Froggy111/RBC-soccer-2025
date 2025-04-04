#include <cstdarg>
#include <cstdio>
#include "debug.hpp"

namespace debug {

// Define color codes
constexpr const char* GREY = "\033[90m";
constexpr const char* BROWN = "\033[33m"; // Yellow-brown
constexpr const char* BLUE = "\033[34m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* RED = "\033[31m";
constexpr const char* BRIGHT_RED = "\033[91m";
constexpr const char* RESET = "\033[0m";

void log(const char *format, ...) {
    #ifndef NO_LOG
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("%s[LOG - RPI]%s %s\n", GREY, RESET, buffer);
    va_end(args);
    #endif
}

void debug(const char *format, ...) {
    #ifdef DEBUG
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("%s[DEBUG - RPI]%s %s\n", BROWN, RESET, buffer);
    va_end(args);
    #endif
}

void info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("%s[INFO - RPI]%s %s\n", BLUE, RESET, buffer);
    va_end(args);
}

void warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("%s[WARN - RPI]%s %s\n", YELLOW, RESET, buffer);
    va_end(args);
}

void error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    fprintf(stderr, "%s[ERROR - RPI]%s %s\n", RED, RESET, buffer);
    va_end(args);
}

void fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    fprintf(stderr, "%s[FATAL - RPI]%s %s\n", BRIGHT_RED, RESET, buffer);
    va_end(args);
}
} // namespace debug