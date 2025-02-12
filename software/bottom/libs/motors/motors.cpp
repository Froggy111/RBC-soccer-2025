#include "motors.hpp"

void Motors::init() {
  motor1.init(1, 115200);
  motor2.init(2, 115200);
  motor3.init(3, 115200);
  motor4.init(4, 115200);
}