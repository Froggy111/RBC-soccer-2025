#include "motors.hpp"
#include "pico/stdlib.h"
#include "libs/hardware-descriptors/pinmap.hpp"
#define MOTOR1
#define MOTOR2
#define MOTOR3
#define MOTOR4

void Motors::init() {
  #ifdef MOTOR1
  motor1.init(1, 115200);
  #endif

  #ifdef MOTOR2
  motor2.init(2, 115200);
  #endif

  #ifdef MOTOR3
  motor3.init(3, 115200);
  #endif

  #ifdef MOTOR4
  motor4.init(4, 115200);
  #endif
}

void Motors::set_error_callback() {
  gpio_set_irq_enabled_with_callback((uint) (pinmap::Pins::DRV1_NFAULT), GPIO_IRQ_EDGE_FALL, true, handle_error_callback);
}

void Motors::handle_error_callback(uint gpio, uint32_t events) {
  switch (gpio) {
    #ifdef MOTOR1
    case (uint) (pinmap::Pins::DRV1_NFAULT):
      motor1.handle_error();
      break;
    #endif

    #ifdef MOTOR2
    case (uint) (pinmap::Pins::DRV2_NFAULT):
      motor2.handle_error();
      break;
    #endif

    #ifdef MOTOR3
    case (uint) (pinmap::Pins::DRV3_NFAULT):
      motor3.handle_error();
      break;
    #endif

    #ifdef MOTOR4
    case (uint) (pinmap::Pins::DRV4_NFAULT):
      motor4.handle_error();
      break;
    #endif
  }
}