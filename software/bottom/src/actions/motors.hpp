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
} __attribute__((packed));

TaskHandle_t motor_task_handle = nullptr;
driver::MotorDriver driver1, driver2, driver3, driver4;
MotorRecvData motor_task_data = {};
u8 motor_task_buffer[sizeof(motor_task_data)];
SemaphoreHandle_t motor_data_mutex = nullptr;

i16 current_duty_cycles[4] = {0};
i16 target_duty_cycles[4] = {0};
const u32 max_duty_cycle = 12500;
const u32 min_duty_cycle_ramp = 10000;
const u32 max_duty_cycle_per_ms = max_duty_cycle / min_duty_cycle_ramp;

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
  vTaskDelay(pdMS_TO_TICKS(1));

  if (driver2.init(2, spi0)) {
    debug::info("Motor Driver 2 Initialized!\n");
  } else {
    debug::error("Motor Driver 2 Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  vTaskDelay(pdMS_TO_TICKS(1));

  if (driver3.init(3, spi0)) {
    debug::info("Motor Driver 3 Initialized!\n");
  } else {
    debug::error("Motor Driver 3 Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  vTaskDelay(pdMS_TO_TICKS(1));

  if (driver4.init(4, spi0)) {
    debug::info("Motor Driver 4 Initialized!\n");
  } else {
    debug::error("Motor Driver 4 Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  vTaskDelay(pdMS_TO_TICKS(1));

  debug::info("Motors initialized");

  TickType_t previous_wait_time = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(1));
    previous_wait_time = xTaskGetTickCount();
    // * data transfer
    if (ulTaskNotifyTake(pdTRUE, 0)) {
      xSemaphoreTake(motor_data_mutex, portMAX_DELAY);
      memcpy(&motor_task_data, motor_task_buffer, sizeof(motor_task_data));
      memset(motor_task_buffer, 0, sizeof(motor_task_buffer));
      xSemaphoreGive(motor_data_mutex);
      target_duty_cycles[motor_task_data.id + 1] = motor_task_data.duty_cycle;
    }
    for (u8 id = 0; id < 4; id++) {
      if (target_duty_cycles[id] != current_duty_cycles[id]) {
        if (target_duty_cycles[id] - current_duty_cycles[id] > 0) {
          current_duty_cycles[id] +=
              MIN(max_duty_cycle_per_ms,
                  target_duty_cycles[id] - current_duty_cycles[id]);
        } else {
          current_duty_cycles[id] -=
              MIN(max_duty_cycle_per_ms,
                  current_duty_cycles[id] - target_duty_cycles[id]);
        }
        switch (id) {
        case 1:
          driver1.command(current_duty_cycles[id]);
        case 2:
          driver2.command(current_duty_cycles[id]);
        case 3:
          driver3.command(current_duty_cycles[id]);
        case 4:
          driver4.command(current_duty_cycles[id]);
        }
      }
    }
    // debug::info("Motor %d: %d\n", motor_task_data.id,
    // motor_task_data.duty_cycle);
  }
}
