#pragma once

#include "comms.hpp"
#include "ICM20948.hpp"
#include "config.hpp"
#include "types.hpp"

// Global IMU configurations and data
icm20948::config_t imu_config1, imu_config2;
icm20948::data_t imu_data;

TaskHandle_t imu_poll_task_handle = nullptr;
struct IMUSendData {
  types::i16 accel_1[3], gyro_1[3], accel_2[3], gyro_2[3];
};

// Task to read and display IMU data
void imu_poll_task(void *args) {
  comms::USB_CDC.printf("Initializing IMU...\r\n");

  // Set IMU Config
  imu_config1.spi = spi0;
  imu_config1.id = 1;
  imu_config2.spi = spi0;
  imu_config2.id = 2;

  // Initialize Both
  if (icm20948::init(&imu_config1) == -1) {
    comms::USB_CDC.printf("Error initializing ICM20948 1.\r\n");
    vTaskDelete(imu_poll_task_handle);
  } else {
    comms::USB_CDC.printf("Initialized IMU20948 1.\r\n");
  }

  if (icm20948::init(&imu_config2) == -1) {
    comms::USB_CDC.printf("Error initializing ICM20948 1.\r\n");
    vTaskDelete(imu_poll_task_handle);
  } else {
    comms::USB_CDC.printf("Initialized IMU20948 1.\r\n");
  }

  IMUSendData to_send = {0};
  comms::USB_CDC.printf("IMU Ready!\r\n");

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

    vTaskDelayUntil(&previous_wait_time, pdMS_TO_TICKS(POLL_INTERVAL));
  }
}
