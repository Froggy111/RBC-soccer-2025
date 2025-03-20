#pragma once

#include "types.hpp"
#include "comms.hpp"
#include "DRV8244.hpp"
#include "projdefs.h"

using namespace types;

#define MOTOR_COUNT 4

struct MotorRecvData {
  uint8_t id;
  uint16_t duty_cycle;
};

TaskHandle_t motor_task_handle = nullptr;
MotorDriver motor_drivers[MOTOR_COUNT];
MotorRecvData motor_task_data = {};
u8 motor_task_buffer[sizeof(motor_task_data)];
SemaphoreHandle_t motor_data_mutex = nullptr;

void motor_task(void *args) {
  // initialize all motors
  for (int i = 0; i < MOTOR_COUNT; i++) {
    motor_drivers[i].init(i, spi0);
    motor_drivers[i].command(0);
  }

  for (;;) {
    // * data transfer
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(motor_data_mutex, portMAX_DELAY);
    memcpy(&motor_task_data, motor_task_buffer,
           sizeof(motor_task_data));
    memset(motor_task_buffer, 0, sizeof(motor_task_buffer));
    xSemaphoreGive(motor_data_mutex);

    motor_drivers[motor_task_data.id].command(motor_task_data.duty_cycle);
  }
}