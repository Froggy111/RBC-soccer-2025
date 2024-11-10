#pragma once
#include <string>
#include <Arduino.h>

#define STALL_LOOP_TIME 1000 // in milliseconds

namespace debug {

extern HardwareSerial* console_serial;

#ifdef DEBUG
void printf(const char* format, ...);
#else
inline void printf(const std::string &format, ...) {return;}
#endif

void stall();
void stall_msg(const std::string &error_msg);
void crash(const std::string &error_msg);
void assert_msg(bool condition, const std::string &error_msg);
std::string format(const char* format, ...);
std::string get_input(const std::string &msg);
std::string read_input(void);
}
