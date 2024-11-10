#include <string>
#include <stdarg.h>
#include <Arduino.h>
#include "types.hpp"
#include "debug.hpp"

namespace debug {

HardwareSerial* console_serial = &Serial;

#ifdef DEBUG
void printf(const char* format, ...) {
  va_list args0;
  va_start(args0, format);
  va_list args1;
  va_copy(args1, args0);
  types::u32 buff_size = (vsnprintf(nullptr, 0, format, args0) + 1) * sizeof(char);
  va_end(args0);
  char* buffer = (char*) malloc(buff_size);
  vsprintf(buffer, format, args1);
  console_serial->print(buffer);
  va_end(args1);
  return;
}
#endif

void stall() {
  while (true) {
    delay(STALL_LOOP_TIME);
  }
  return;
}

void stall_msg(const std::string &error_msg) {
  while (true) {
    console_serial->print(error_msg.c_str());
    delay(STALL_LOOP_TIME);
  }
  return;
}

void crash(const std::string &error_msg) {
  esp_system_abort(error_msg.c_str());
  return;
}

void assert_msg(bool condition, const std::string &error_msg) {
  if (!condition) {
    esp_system_abort(error_msg.c_str());
  }
  return;
}

std::string format(const char* format, ...) {
  va_list args0;
  va_start(args0, format);
  va_list args1;
  va_copy(args1, args0);
  types::u32 buff_size = (vsnprintf(nullptr, 0, format, args0) + 1) * sizeof(char);
  va_end(args0);
  char* buffer = (char*) malloc(buff_size);
  vsprintf(buffer, format, args1);
  return buffer;
}

std::string get_input(const std::string &msg) {
  console_serial->print(msg.c_str());
  while (!console_serial->available()) {
    delay(1);
  }
  std::string input = console_serial->readString().c_str();
  input.erase(input.find_last_not_of("\r\n") + 1);
  return input;
}

std::string read_input(void) {
  if (console_serial->available()) {
    std::string input = console_serial->readString().c_str();
    input.erase(input.find_last_not_of("\r\n") + 1);
    return input;
  }
  else {
    return std::string();
  }
}

}
