#pragma once

#include "types.hpp"
#include "comms.hpp"
#include "DRV8244.hpp"
#include "projdefs.h"
#include "debug.hpp"

using namespace types;

#define MOTOR_COUNT 4

struct MotorRecvData {
  uint8_t id;
  i16 duty_cycle;
};

static TaskHandle_t motor_task_handle = nullptr;
static driver::MotorDriver driver1;
static driver::MotorDriver driver2;
static driver::MotorDriver driver3;
static driver::MotorDriver driver4;
static MotorRecvData motor_task_data = {};
static u8 motor_task_buffer[sizeof(motor_task_data)];
static SemaphoreHandle_t motor_data_mutex = nullptr;

void motor_task(void *args) {
  // initialize all motors
  if (driver1.init(1, spi0)) {
    debug::info("Motor Driver 1 Initialized!\n");
  } else {
    debug::error("Motor Driver 1 Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver2.init(2, spi0)) {
    debug::info("Motor Driver 2 Initialized!\n");
  } else {
    debug::error("Motor Driver 2 Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver3.init(3, spi0)) {
    debug::info("Motor Driver 3 Initialized!\n");
  } else {
    debug::error("Motor Driver 3 Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver4.init(4, spi0)) {
    debug::info("Motor Driver 4 Initialized!\n");
  } else {
    debug::error("Motor Driver 4 Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  debug::info("Motors initialized");

  for (;;) {
    // * data transfer
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(motor_data_mutex, portMAX_DELAY);
    memcpy(&motor_task_data, motor_task_buffer, sizeof(motor_task_data));
    memset(motor_task_buffer, 0, sizeof(motor_task_buffer));
    xSemaphoreGive(motor_data_mutex);

    switch (motor_task_data.id) {
      case 1:
        driver1.command(motor_task_data.duty_cycle);
        break;
      case 2:
        driver2.command(motor_task_data.duty_cycle);
        break;
      case 3:
        driver3.command(motor_task_data.duty_cycle);
        break;
      case 4:
        driver4.command(motor_task_data.duty_cycle);
        break;
      default:
        debug::error("Invalid motor ID: %d\n", motor_task_data.id);
        break;
    }
    debug::info("Motor %d: %d\n", motor_task_data.id,
                motor_task_data.duty_cycle);
  }
}
