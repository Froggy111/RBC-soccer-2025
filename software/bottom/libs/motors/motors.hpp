#pragma once
#include "libs/motors/DRV8244.hpp"

class Motors {
private:
  static MotorDriver motor1;
  static MotorDriver motor2;
  static MotorDriver motor3;
  static MotorDriver motor4;
public:
  void init();
  void set_error_callback();
  static void handle_error_callback(uint gpio, uint32_t events);
};