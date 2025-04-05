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

static TaskHandle_t motor_task_handle = nullptr;
static driver::MotorDriver driver1;
static driver::MotorDriver driver2;
static driver::MotorDriver driver3;
static driver::MotorDriver driver4;
static MotorRecvData motor_task_data = {};
static u8 motor_task_buffer[sizeof(motor_task_data)];
static SemaphoreHandle_t motor_data_mutex = nullptr;

// static i16 current_duty_cycles[4] = {0};
// static i16 target_duty_cycles[4] = {0};
// const u32 max_duty_cycle = 12500;
// const u32 min_duty_cycle_ramp = 10000;
// const u32 max_duty_cycle_per_ms = max_duty_cycle / min_duty_cycle_ramp;

void motor_task(void *args) {
  // comms::USB_CDC.wait_for_CDC_connection();
  if (driver1.init(1, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver3.init(3, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver4.init(4, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  if (driver2.init(2, spi0)) {
    driver1.set_ITRIP(driver::ITRIP::ITRIP::TRIP_2_97V);
    driver1.set_OCP(driver::OCP::OCP::SETTING_100);
    debug::log("Motor Driver Initialized!\n");
  } else {
    debug::log("Motor Driver Initialization Failed!\n");
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  debug::info("Motors initialized");
  for (;;) {
    // driver1.command(2000);
    // driver2.command(2000);
    // driver3.command(2000);
    // driver4.command(2000);
    // vTaskDelay(pdMS_TO_TICKS(1));
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
  }
}
