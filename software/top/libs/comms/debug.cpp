#include "comms.hpp"
#include "debug.hpp"
#include <cstdarg>

namespace debug {

#ifdef DEBUG

void log(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string
  char buffer[256]; // Reasonable size for debug messages
  vsnprintf(buffer, sizeof(buffer), format, args);

  // Use the CDC printf for debug mode
  usb::CDC::printf("%s", buffer);

  va_end(args);
}

void debug(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string
  char buffer[256];
  int prefix_len = snprintf(buffer, sizeof(buffer), "[DEBUG] ");
  vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);

  // Use the CDC printf for debug mode
  usb::CDC::printf("%s", buffer);

  va_end(args);
}

void info(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string
  char buffer[256];
  int prefix_len = snprintf(buffer, sizeof(buffer), "[INFO] ");
  vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);

  // Use the CDC printf for debug mode
  usb::CDC::printf("%s", buffer);

  va_end(args);
}

void warn(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string
  char buffer[256];
  int prefix_len = snprintf(buffer, sizeof(buffer), "[WARN] ");
  vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);

  // Use the CDC printf for debug mode
  usb::CDC::printf("%s", buffer);

  va_end(args);
}

void error(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string
  char buffer[256];
  int prefix_len = snprintf(buffer, sizeof(buffer), "[ERROR] ");
  vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);

  // Use the CDC printf for debug mode
  usb::CDC::printf("%s", buffer);

  va_end(args);
}

void fatal(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string
  char buffer[256];
  int prefix_len = snprintf(buffer, sizeof(buffer), "[FATAL] ");
  vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);

  // Use the CDC printf for debug mode
  usb::CDC::printf("%s", buffer);

  va_end(args);
}

#else

void log(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string with prefix
  char buffer[256];
  const char *prefix = "[LOG - TOP_PICO] ";

  // Copy prefix to buffer
  int prefix_len = snprintf(buffer, sizeof(buffer), "%s", prefix);

  // Format the message after the prefix
  int msg_len =
      vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
  int total_len = prefix_len + msg_len;

  // Use CDC write with LOG identifier for non-debug mode
  usb::CDC::write(comms::SendIdentifiers::COMMS_DEBUG,
                  (const types::u8 *)buffer,
                  static_cast<types::u16>(total_len));

  va_end(args);
}

void debug(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string with prefix
  char buffer[256];
  const char *prefix = "[DEBUG - TOP_PICO] ";

  // Copy prefix to buffer
  int prefix_len = snprintf(buffer, sizeof(buffer), "%s", prefix);

  // Format the message after the prefix
  int msg_len =
      vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
  int total_len = prefix_len + msg_len;

  // Use CDC write with LOG identifier for non-debug mode
  usb::CDC::write(comms::SendIdentifiers::COMMS_DEBUG,
                  (const types::u8 *)buffer,
                  static_cast<types::u16>(total_len));

  va_end(args);
}

void info(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string with prefix
  char buffer[256];
  const char *prefix = "[INFO - TOP_PICO] ";

  // Copy prefix to buffer
  int prefix_len = snprintf(buffer, sizeof(buffer), "%s", prefix);

  // Format the message after the prefix
  int msg_len =
      vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
  int total_len = prefix_len + msg_len;

  // Use CDC write with LOG identifier for non-debug mode
  usb::CDC::write(comms::SendIdentifiers::COMMS_DEBUG,
                  (const types::u8 *)buffer,
                  static_cast<types::u16>(total_len));

  va_end(args);
}

void warn(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string with prefix
  char buffer[256];
  const char *prefix = "[WARN - TOP_PICO] ";

  // Copy prefix to buffer
  int prefix_len = snprintf(buffer, sizeof(buffer), "%s", prefix);

  // Format the message after the prefix
  int msg_len =
      vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
  int total_len = prefix_len + msg_len;

  // Use CDC write with LOG identifier for non-debug mode
  usb::CDC::write(comms::SendIdentifiers::COMMS_DEBUG,
                  (const types::u8 *)buffer,
                  static_cast<types::u16>(total_len));

  va_end(args);
}

void error(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string with prefix
  char buffer[256];
  const char *prefix = "[ERROR - TOP_PICO] ";

  // Copy prefix to buffer
  int prefix_len = snprintf(buffer, sizeof(buffer), "%s", prefix);

  // Format the message after the prefix
  int msg_len =
      vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
  int total_len = prefix_len + msg_len;

  // Use CDC write with LOG identifier for non-debug mode
  usb::CDC::write(comms::SendIdentifiers::COMMS_DEBUG,
                  (const types::u8 *)buffer,
                  static_cast<types::u16>(total_len));

  va_end(args);
}

void fatal(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // Buffer for formatted string with prefix
  char buffer[256];
  const char *prefix = "[FATAL - TOP_PICO] ";

  // Copy prefix to buffer
  int prefix_len = snprintf(buffer, sizeof(buffer), "%s", prefix);

  // Format the message after the prefix
  int msg_len =
      vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
  int total_len = prefix_len + msg_len;

  // Use CDC write with LOG identifier for non-debug mode
  usb::CDC::write(comms::SendIdentifiers::COMMS_DEBUG,
                  (const types::u8 *)buffer,
                  static_cast<types::u16>(total_len));

  va_end(args);
}

#endif

} // namespace debug
