#pragma once
#include "libs/motors/DRV8244.hpp"

class Motors {
private:
  MotorDriver motor1;
  MotorDriver motor2;
  MotorDriver motor3;
  MotorDriver motor4;
public:
  void init();
  void on_error_callback();
};