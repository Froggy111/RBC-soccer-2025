#include <string>
#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "comms.hpp"
#include "debug.hpp"

namespace debug {

void log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cout << "[LOG] " << buffer << std::endl;
    va_end(args);
}

void debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cout << "[DEBUG] " << buffer << std::endl;
    va_end(args);
}

void info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cout << "[INFO] " << buffer << std::endl;
    va_end(args);
}

void warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cout << "[WARN] " << buffer << std::endl;
    va_end(args);
}

void error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cerr << "[ERROR] " << buffer << std::endl;
    va_end(args);
}

void fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cerr << "[FATAL] " << buffer << std::endl;
    va_end(args);
}

void msg(std::string format, LogLevel log_level, ...) {
    va_list args;
    va_start(args, log_level);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    
    switch (log_level) {
        case LogLevel::TRACE:
            std::cout << "[TRACE] " << buffer << std::endl;
            break;
        case LogLevel::DEBUG:
            std::cout << "[DEBUG] " << buffer << std::endl;
            break;
        case LogLevel::INFO:
            std::cout << "[INFO] " << buffer << std::endl;
            break;
        case LogLevel::WARN:
            std::cout << "[WARN] " << buffer << std::endl;
            break;
        case LogLevel::ERROR:
            std::cerr << "[ERROR] " << buffer << std::endl;
            break;
        case LogLevel::FATAL:
            std::cerr << "[FATAL] " << buffer << std::endl;
            break;
    }
    
    va_end(args);
}

void msg_device(const usb::USBDevice& device, std::string format, LogLevel log_level, ...) {
    va_list args;
    va_start(args, log_level);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    
    std::string device_info = "[" + device.deviceNode + "] ";
    
    switch (log_level) {
        case LogLevel::TRACE:
            std::cout << "[TRACE] " << device_info << buffer << std::endl;
            break;
        case LogLevel::DEBUG:
            std::cout << "[DEBUG] " << device_info << buffer << std::endl;
            break;
        case LogLevel::INFO:
            std::cout << "[INFO] " << device_info << buffer << std::endl;
            break;
        case LogLevel::WARN:
            std::cout << "[WARN] " << device_info << buffer << std::endl;
            break;
        case LogLevel::ERROR:
            std::cerr << "[ERROR] " << device_info << buffer << std::endl;
            break;
        case LogLevel::FATAL:
            std::cerr << "[FATAL] " << device_info << buffer << std::endl;
            break;
    }
    
    va_end(args);
}

} // namespace debug