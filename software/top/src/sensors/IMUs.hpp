#pragma once

#include "comms.hpp"
#include "debug.hpp"
#include "ICM20948.hpp"
#include "config.hpp"
#include "types.hpp"

// Global IMU configurations and data
icm20948::config_t imu_config1 = {1, spi0};
icm20948::config_t imu_config2 = {2, spi0};
icm20948::data_t imu_data;

TaskHandle_t imu_poll_task_handle = nullptr;
struct IMUSendData {
  types::i16 accel_1[3], gyro_1[3], accel_2[3], gyro_2[3];
};

// Task to read and display IMU data
void imu_poll_task(void *args) {
  debug::log("Initializing IMU...\r\n");

  // Initialize Both
  if (icm20948::init(&imu_config1) == -1) {
    debug::log("Error initializing IMU 1.\r\n");
    vTaskDelete(imu_poll_task_handle);
  } else {
    debug::log("Initialized IMU 1.\r\n");
  }

  if (icm20948::init(&imu_config2) == -1) {
    debug::log("Error initializing IMU 2.\r\n");
    vTaskDelete(imu_poll_task_handle);
  } else {
    debug::log("Initialized IMU 2.\r\n");
  }

  // Set IMU Config
  debug::log("set IMU1 config\r\n");
  icm20948::set_accel_config(&imu_config1, 1000.0f / IMU_POLL_INTERVAL, 2,
                             IMU_FSR, false);
  icm20948::set_accel_config(&imu_config1, 1000.0f / IMU_POLL_INTERVAL, 2,
                             IMU_FSR, false);
  icm20948::set_gyro_config(&imu_config1, 1000.0f / IMU_POLL_INTERVAL, 2,
                            false);
  icm20948::set_gyro_config(&imu_config1, 1000.0f / IMU_POLL_INTERVAL, 2,
                            false);

  IMUSendData to_send = {0};
  debug::log("IMU Ready!\r\n");

  for (;;) {
    TickType_t previous_wait_time = xTaskGetTickCount();

    // read both
    icm20948::read_raw_accel(&imu_config1, to_send.accel_1);
    icm20948::read_raw_gyro(&imu_config1, to_send.gyro_1);
    icm20948::read_raw_accel(&imu_config2, to_send.accel_2);
    icm20948::read_raw_gyro(&imu_config2, to_send.accel_2);

    // send both
    comms::USB_CDC.write(comms::SendIdentifiers::IMU, (types::u8 *)&to_send,
                         sizeof(to_send));

    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(IMU_POLL_INTERVAL));
  }
}
