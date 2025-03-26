#include <string>
#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "comms.hpp"
#include "debug.hpp"

namespace debug {

void log(char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
  fflush(stdout);
  
  // Also send over USB
  if (comms::USB_CDC.device_connected()) {
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    comms::USB_CDC.write(comms::SendIdentifiers::COMMS_DEBUG, 
                        (types::u8*)buffer, strlen(buffer));
  }
}

void debug(char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stdout, "[DEBUG] ");
  vfprintf(stdout, format, args);
  va_end(args);
  fflush(stdout);
  
  if (comms::USB_CDC.device_connected()) {
    va_start(args, format);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[DEBUG] ");
    vsnprintf(buffer + 8, sizeof(buffer) - 8, format, args);
    va_end(args);
    
    comms::USB_CDC.write(comms::SendIdentifiers::COMMS_DEBUG, 
                        (types::u8*)buffer, strlen(buffer));
  }
}

void info(char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stdout, "[INFO] ");
  vfprintf(stdout, format, args);
  va_end(args);
  fflush(stdout);
  
  if (comms::USB_CDC.device_connected()) {
    va_start(args, format);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[INFO] ");
    vsnprintf(buffer + 7, sizeof(buffer) - 7, format, args);
    va_end(args);
    
    comms::USB_CDC.write(comms::SendIdentifiers::COMMS_DEBUG, 
                        (types::u8*)buffer, strlen(buffer));
  }
}

void warn(char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "[WARN] ");
  vfprintf(stderr, format, args);
  va_end(args);
  fflush(stderr);
  
  if (comms::USB_CDC.device_connected()) {
    va_start(args, format);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[WARN] ");
    vsnprintf(buffer + 7, sizeof(buffer) - 7, format, args);
    va_end(args);
    
    comms::USB_CDC.write(comms::SendIdentifiers::COMMS_WARN, 
                        (types::u8*)buffer, strlen(buffer));
  }
}

void error(char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "[ERROR] ");
  vfprintf(stderr, format, args);
  va_end(args);
  fflush(stderr);
  
  if (comms::USB_CDC.device_connected()) {
    va_start(args, format);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[ERROR] ");
    vsnprintf(buffer + 8, sizeof(buffer) - 8, format, args);
    va_end(args);
    
    comms::USB_CDC.write(comms::SendIdentifiers::COMMS_ERROR, 
                        (types::u8*)buffer, strlen(buffer));
  }
}

void fatal(char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "[FATAL] ");
  vfprintf(stderr, format, args);
  va_end(args);
  fflush(stderr);
  
  if (comms::USB_CDC.device_connected()) {
    va_start(args, format);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[FATAL] ");
    vsnprintf(buffer + 8, sizeof(buffer) - 8, format, args);
    va_end(args);
    
    comms::USB_CDC.write(comms::SendIdentifiers::COMMS_ERROR, 
                        (types::u8*)buffer, strlen(buffer));
  }
}

void msg(std::string format, LogLevel log_level, ...) {
  va_list args;
  va_start(args, log_level);
  
  switch (log_level) {
    case LogLevel::TRACE:
      fprintf(stdout, "[TRACE] ");
      break;
    case LogLevel::DEBUG:
      fprintf(stdout, "[DEBUG] ");
      break;
    case LogLevel::INFO:
      fprintf(stdout, "[INFO] ");
      break;
    case LogLevel::WARN:
      fprintf(stderr, "[WARN] ");
      break;
    case LogLevel::ERROR:
      fprintf(stderr, "[ERROR] ");
      break;
    case LogLevel::FATAL:
      fprintf(stderr, "[FATAL] ");
      break;
  }
  
  vfprintf(log_level > LogLevel::INFO ? stderr : stdout, format.c_str(), args);
  va_end(args);
  fflush(log_level > LogLevel::INFO ? stderr : stdout);
}

void msg_UART(std::string format, LogLevel log_level, ...) {
  // This function can be implemented when UART communication is added
}

} // namespace debug