#include "ICM20948.hpp"
#include "comms.hpp"
#include "config.hpp"

icm20948::config_t imu_config1, imu_config2;
icm20948::data_t imu_data;

TaskHandle_t imu_poll_task_handle = nullptr;

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

  // Set magnetometer update rate (mode 0-4, where higher is faster)
  icm20948::set_mag_rate(&imu_config1, 3);
  icm20948::set_mag_rate(&imu_config2, 3);

  int16_t accel[3], gyro[3];

  int16_t accel_bias1[3] = {0}, gyro_bias1[3] = {0};
  int16_t accel_bias2[3] = {0}, gyro_bias2[3] = {0};
  int16_t raw_temp;
  float temp_c;

  // Calibrate sensors
  comms::USB_CDC.printf("Calibrating ICM20948s...\r\n");
  icm20948::cal_gyro(&imu_config1, gyro_bias1);
  // comms::USB_CDC.printf("Gyro bias: [%d, %d, %d]\r\n", gyro_bias[0],
  //                       gyro_bias[1], gyro_bias[2]);
  icm20948::cal_accel(&imu_config1, accel_bias1);
  // comms::USB_CDC.printf("Accel bias: [%d, %d, %d]\r\n", accel_bias[0],
  //                       accel_bias[1], accel_bias[2]);

  icm20948::cal_gyro(&imu_config2, gyro_bias2);
  // comms::USB_CDC.printf("Gyro bias: [%d, %d, %d]\r\n", gyro_bias[0],
  //                       gyro_bias[1], gyro_bias[2]);
  icm20948::cal_accel(&imu_config2, accel_bias2);
  // comms::USB_CDC.printf("Accel bias: [%d, %d, %d]\r\n", accel_bias[0],
  //                       accel_bias[1], accel_bias[2]);

  comms::USB_CDC.printf("IMU Ready!\r\n");

  while (true) {
    // read ICM20948 1
    icm20948::read_cal_accel(&imu_config1, accel, accel_bias1);
    icm20948::read_cal_gyro(&imu_config1, gyro, gyro_bias1);

    // send ICM20948 2
    comms::USB_CDC.write(comms::SendIdentifiers::ICM29048, &to_send, sizeof to_send);

    // read ICM20948 2
    icm20948::read_cal_accel(&imu_config2, accel, accel_bias2);
    icm20948::read_cal_gyro(&imu_config2, gyro, gyro_bias2);

    // send ICM20948 2
    comms::USB_CDC.write(comms::SendIdentifiers::ICM29048, &to_send, sizeof to_send);



    // Wait before next reading
    vTaskDelay(pdMS_TO_TICKS(TopPlateConfig.refresh_rate));
  }
}