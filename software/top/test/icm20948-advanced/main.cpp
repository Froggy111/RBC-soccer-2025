#include <cstdint>
#include <hardware/spi.h>
#include <hardware/structs/io_bank0.h>
#include "comms.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include "ICM20948.hpp"

extern "C" {
#include <pico/stdlib.h>
}

using namespace types;

// Configuration
const u8 LED_PIN = 25;

// Global variables
icm20948::config_t imu_config1;
icm20948::config_t imu_config2;

// Task to read and display IMU data
void imu_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);
  comms::USB_CDC.printf("ICM20948 IMU Test\r\n");

  if (!spi_init(spi0, 4000000)) {
    comms::USB_CDC.printf("Error initializing SPI\r\n");
    while (1) {
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
    }
  } else {
    comms::USB_CDC.printf("SPI initialized!\r\n");
  }

  comms::USB_CDC.printf("Initializing IMU...\r\n");

  // Initialize IMU
  int8_t result1 = icm20948::init(&imu_config1);
  int8_t result2 = icm20948::init(&imu_config2);
  if (result1 != 0 || result2 != 0) {
    comms::USB_CDC.printf("Error initializing ICM20948, Error Code: %d %d\r\n",
                          result1, result2);
    while (1) {
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
    }
  } else {
    comms::USB_CDC.printf("ICM20948 initialized!! :D\r\n");
  }

  // Set magnetometer update rate (mode 0-4, where higher is faster)
  icm20948::set_mag_rate(&imu_config1, 2);
  icm20948::set_mag_rate(&imu_config2, 2);

  int16_t accel1[3], gyro1[3];
  int16_t accel2[3], gyro2[3];
  int16_t accel_bias1[3] = {0}, gyro_bias1[3] = {0};
  int16_t accel_bias2[3] = {0}, gyro_bias2[3] = {0};

  // Calibrate sensors (this may take some time)
  comms::USB_CDC.printf("Calibrating gyroscope 1...\r\n");
  icm20948::cal_gyro(&imu_config1, gyro_bias1);
  comms::USB_CDC.printf("Gyro bias: [%d, %d, %d]\r\n", gyro_bias1[0],
                        gyro_bias1[1], gyro_bias1[2]);

  comms::USB_CDC.printf("Calibrating accelerometer 1...\r\n");
  icm20948::cal_accel(&imu_config1, accel_bias1);
  comms::USB_CDC.printf("Accel bias: [%d, %d, %d]\r\n", accel_bias1[0],
                        accel_bias1[1], accel_bias1[2]);

  comms::USB_CDC.printf("Calibrating gyroscope 2...\r\n");
  icm20948::cal_gyro(&imu_config2, gyro_bias2);
  comms::USB_CDC.printf("Gyro bias: [%d, %d, %d]\r\n", gyro_bias2[0],
                        gyro_bias2[1], gyro_bias2[2]);

  comms::USB_CDC.printf("Calibrating accelerometer 2...\r\n");
  icm20948::cal_accel(&imu_config2, accel_bias2);
  comms::USB_CDC.printf("Accel bias: [%d, %d, %d]\r\n", accel_bias2[0],
                        accel_bias2[1], accel_bias2[2]);

  comms::USB_CDC.printf("Starting IMU readings...\r\n");

  while (true) {
    // Toggle LED to indicate activity
    gpio_put(LED_PIN, !gpio_get(LED_PIN));

    // Read calibrated sensor values
    icm20948::read_cal_accel(&imu_config1, accel1, accel_bias1);
    icm20948::read_cal_gyro(&imu_config1, gyro1, gyro_bias1);

    icm20948::read_cal_accel(&imu_config2, accel2, accel_bias2);
    icm20948::read_cal_gyro(&imu_config2, gyro2, gyro_bias2);

    // flip accel2 and gyro2
    accel2[0] = -accel2[0];
    accel2[1] = -accel2[1];

    gyro2[0] = -gyro2[0];
    gyro2[1] = -gyro2[1];
    gyro2[2] = -gyro2[2];

    // Combined Sensor Outputs
    int combined_accel[3] = {((accel1[0] - accel2[0]) / 2),
                             ((accel1[1] - accel2[1]) / 2),
                             ((accel1[2] + accel2[2]) / 2)};
    int combined_gyro[3] = {((gyro1[0] + gyro2[0]) / 2),
                            ((gyro1[1] + gyro2[1]) / 2),
                            ((gyro1[2] + gyro2[2]) / 2)};

    // Print sensor data
    comms::USB_CDC.printf("IMU Data:\r\n");
    comms::USB_CDC.printf("IMU 1: Accel: [%d, %d, %d], Gyro: [%d, %d, %d]\r\n",
                          accel1[0], accel1[1], accel1[2], gyro1[0], gyro1[1],
                          gyro1[2]);
    comms::USB_CDC.printf("IMU 2: Accel: [%d, %d, %d], Gyro: [%d, %d, %d]\r\n",
                          accel2[0], accel2[1], accel2[2], gyro2[0], gyro2[1],
                          gyro2[2]);
    comms::USB_CDC.printf(
        "Combined: Accel: [%d, %d, %d], Gyro: [%d, %d, %d]\r\n",
        combined_accel[0], combined_accel[1], combined_accel[2],
        combined_gyro[0], combined_gyro[1], combined_gyro[2]);

    // Wait before next reading
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

int main() {
  // Initialize LED for status indication
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // Initialize CS pin for the other IMU
  gpio_init((uint)pinmap::Pico::IMU2_NCS);
  gpio_set_dir((uint)pinmap::Pico::IMU2_NCS, GPIO_OUT);
  gpio_put((uint)pinmap::Pico::IMU2_NCS, 1);

  // Initialize USB CDC for communication
  comms::init();

  // Configure IMU
  imu_config1.spi = spi0;
  imu_config1.id = 1;

  imu_config2.spi = spi0;
  imu_config2.id = 2;

  // Create task for reading IMU data
  xTaskCreate(imu_task, "imu_task", 2048, NULL, 10, NULL);

  // Start FreeRTOS scheduler
  vTaskStartScheduler();

  return 0;
}
