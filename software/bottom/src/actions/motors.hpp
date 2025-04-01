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
  uint16_t duty_cycle;
};

static TaskHandle_t motor_task_handle = nullptr;
static driver::MotorDriver motor_drivers[MOTOR_COUNT];
static MotorRecvData motor_task_data = {};
static u8 motor_task_buffer[sizeof(motor_task_data)];
static SemaphoreHandle_t motor_data_mutex = nullptr;

void motor_task(void *args) {
  // initialize all motors
  for (int i = 1; i <= MOTOR_COUNT; i++) {
    if (!motor_drivers[i].init(i, spi0)) {
      debug::error("Motor %d failed to initialize\n", i);
      while (true) {
        gpio_put(25, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_put(25, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
      }
    }
    motor_drivers[i].command(0);
  }

  debug::info("Motors initialized");

  for (;;) {
    // * data transfer
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(motor_data_mutex, portMAX_DELAY);
    memcpy(&motor_task_data, motor_task_buffer, sizeof(motor_task_data));
    memset(motor_task_buffer, 0, sizeof(motor_task_buffer));
    xSemaphoreGive(motor_data_mutex);

    motor_drivers[motor_task_data.id].command(motor_task_data.duty_cycle);
    debug::info("Motor %d: %d\n", motor_task_data.id,
                motor_task_data.duty_cycle);
  }
}
