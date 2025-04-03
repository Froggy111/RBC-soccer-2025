#pragma once

#include "types.hpp"
#include "comms.hpp"
#include "DRV8244.hpp"
#include "projdefs.h"
#include "debug.hpp"
#include <cmath>     // For std::abs
#include <algorithm> // For std::min/max

using namespace types;

#define MOTOR_COUNT 4

struct MotorRecvData {
  uint8_t id;
  i16 duty_cycle;
} __attribute__((packed));

TaskHandle_t motor_task_handle = nullptr;
TaskHandle_t motor_ramp_task_handle = nullptr; // Handle for the new ramp task
driver::MotorDriver driver1, driver2, driver3, driver4;
MotorRecvData motor_task_data = {};
u8 motor_task_buffer[sizeof(motor_task_data)];
SemaphoreHandle_t motor_data_mutex = nullptr;
const u32 min_zero_to_full_speed_ms = MOTOR_MIN_ZERO_TO_FULL_SPEED_TIME;

// Assuming DRV8244 uses a range like -12500 to +12500 for duty cycle
const i16 max_abs_duty_cycle =
    12500; // Adjust if the driver uses a different range
const u32 ramp_task_interval_ms =
    10; // How often the ramp task runs and updates motors
// Calculate max change per step based on ramp task interval
const i32 max_duty_cycle_change_per_step =
    static_cast<i32>(max_abs_duty_cycle) * ramp_task_interval_ms /
    min_zero_to_full_speed_ms;

// Shared state between motor_task and motor_ramp_task
i16 target_duty_cycles[MOTOR_COUNT] = {0};  // Target duty cycle for each motor
i16 current_duty_cycles[MOTOR_COUNT] = {0}; // Current actual duty cycle applied

// New task to handle duty cycle ramping and actual motor commanding
void motor_ramp_task(void *args) {
  TickType_t last_wake_time = xTaskGetTickCount();

  for (;;) {
    xSemaphoreTake(motor_data_mutex, portMAX_DELAY);

    for (int i = 0; i < MOTOR_COUNT; ++i) {
      i16 target = target_duty_cycles[i];
      i16 current = current_duty_cycles[i];
      i16 new_duty_cycle = current;
      i32 diff = static_cast<i32>(target) - static_cast<i32>(current);

      if (std::abs(diff) <= max_duty_cycle_change_per_step) {
        // Small difference, jump directly to target
        new_duty_cycle = target;
      } else if (diff > 0) {
        // Need to increase duty cycle
        new_duty_cycle = current + max_duty_cycle_change_per_step;
      } else { // diff < 0
        // Need to decrease duty cycle
        new_duty_cycle = current - max_duty_cycle_change_per_step;
      }

      // Clamp to valid duty cycle range if necessary (assuming symmetrical range)
      // new_duty_cycle = std::max(static_cast<i16>(-max_abs_duty_cycle), std::min(max_abs_duty_cycle, new_duty_cycle));

      current_duty_cycles[i] =
          new_duty_cycle; // Update the current actual duty cycle

      // Command the motor with the calculated step
      switch (i) {
      case 0: // Corresponds to motor ID 1
        driver1.command(new_duty_cycle);
        break;
      case 1: // Corresponds to motor ID 2
        driver2.command(new_duty_cycle);
        break;
      case 2: // Corresponds to motor ID 3
        driver3.command(new_duty_cycle);
        break;
      case 3: // Corresponds to motor ID 4
        driver4.command(new_duty_cycle);
        break;
      }
    }

    xSemaphoreGive(motor_data_mutex);

    // Delay until the next ramp interval
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(ramp_task_interval_ms));
  }
}

// Original task, now only updates target duty cycles
void motor_task(void *args) {
  // initialize all motors
  // NOTE: Mutex must be created before this task starts if ramp task also uses it
  //       Or, ensure ramp task doesn't run until drivers are initialized.
  //       Assuming mutex is created elsewhere before tasks are started.

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

  // *** Important: Create and start the motor_ramp_task *after* drivers are initialized ***
  // xTaskCreate(motor_ramp_task, "Motor Ramp Task", RAMP_TASK_STACK_SIZE, NULL, RAMP_TASK_PRIORITY, &motor_ramp_task_handle);

  for (;;) {
    // * data transfer (receive target duty cycle)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(motor_data_mutex, portMAX_DELAY);
    memcpy(&motor_task_data, motor_task_buffer, sizeof(motor_task_data));
    memset(motor_task_buffer, 0, sizeof(motor_task_buffer));

    // Update the target duty cycle for the specific motor
    if (motor_task_data.id >= 1 && motor_task_data.id <= MOTOR_COUNT) {
      // Store the target duty cycle, ramp task will handle actual commanding
      target_duty_cycles[motor_task_data.id - 1] = motor_task_data.duty_cycle;
      // debug::info("Motor %d target set to: %d\n", motor_task_data.id, motor_task_data.duty_cycle);
    } else {
      debug::error("Invalid motor ID received: %d\n", motor_task_data.id);
    }

    xSemaphoreGive(motor_data_mutex);

    // Removed direct driver command call from this task
    // switch (motor_task_data.id) { ... }
  }
}

// Remember to:
// 1. Create the `motor_data_mutex` (using xSemaphoreCreateMutex()) before starting either task.
// 2. Create the `motor_ramp_task` (using xTaskCreate()) *after* the drivers are successfully initialized in `motor_task` (or ensure motor_task runs first and initializes them before motor_ramp_task starts executing its loop). A simple way is to create the ramp task just before the infinite loop in `motor_task` after initialization succeeds. Provide appropriate stack size and priority.
// 3. Define `MOTOR_MIN_ZERO_TO_FULL_SPEED_TIME` in `projdefs.h` or elsewhere.
// 4. Include `<cmath>` and `<algorithm>`.
