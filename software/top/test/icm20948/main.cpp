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

// Default ICM20948 addresses
const u8 ICM20948_ACCEL_GYRO_ADDR = 0x68; // Default accel/gyro address
const u8 ICM20948_MAG_ADDR = 0x0C;        // Default magnetometer address

// Global variables
icm20948::icm20948_config_t imu_config;
icm20948::icm20984_data_t imu_data;

// Task to read and display IMU data
void imu_task(void *args) {
  comms::USB_CDC.wait_for_CDC_connection(0xFFFFFFFF);
  comms::USB_CDC.printf("ICM20948 IMU Test\r\n");

  if (!spi_init(spi0, 1000000)) {
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
  int8_t result = icm20948::icm20948_init(&imu_config);
  if (result != 0) {
    comms::USB_CDC.printf("Error initializing ICM20948: %d\r\n", result);
    while (1) {
      gpio_put(LED_PIN, 0);
      sleep_ms(100);
      gpio_put(LED_PIN, 1);
      sleep_ms(100);
    }
  } else {
    comms::USB_CDC.printf("ICM20948 initialized\r\n");
  }

  // Set magnetometer update rate (mode 0-4, where higher is faster)
  icm20948::icm20948_set_mag_rate(&imu_config, 2);

  int16_t accel[3], gyro[3], mag[3];
  int16_t accel_bias[3] = {0}, gyro_bias[3] = {0}, mag_bias[3] = {0};
  int16_t raw_temp;
  float temp_c;

  // Calibrate sensors (this may take some time)
  comms::USB_CDC.printf("Calibrating gyroscope...\r\n");
  icm20948::icm20948_cal_gyro(&imu_config, gyro_bias);
  comms::USB_CDC.printf("Gyro bias: [%d, %d, %d]\r\n", gyro_bias[0],
                        gyro_bias[1], gyro_bias[2]);

  comms::USB_CDC.printf("Calibrating accelerometer...\r\n");
  icm20948::icm20948_cal_accel(&imu_config, accel_bias);
  comms::USB_CDC.printf("Accel bias: [%d, %d, %d]\r\n", accel_bias[0],
                        accel_bias[1], accel_bias[2]);

  comms::USB_CDC.printf("Calibrating magnetometer...\r\n");
  icm20948::icm20948_cal_mag_simple(&imu_config, mag_bias);
  comms::USB_CDC.printf("Mag bias: [%d, %d, %d]\r\n", mag_bias[0], mag_bias[1],
                        mag_bias[2]);

  comms::USB_CDC.printf("Starting IMU readings...\r\n");

  while (true) {
    // Toggle LED to indicate activity
    gpio_put(LED_PIN, !gpio_get(LED_PIN));

    // Read calibrated sensor values
    icm20948::icm20948_read_cal_accel(&imu_config, accel, accel_bias);
    icm20948::icm20948_read_cal_gyro(&imu_config, gyro, gyro_bias);
    icm20948::icm20948_read_cal_mag(&imu_config, mag, mag_bias);
    icm20948::icm20948_read_temp_c(&imu_config, &temp_c);

    // Print sensor data
    comms::USB_CDC.printf("IMU Data:\r\n");
    comms::USB_CDC.printf("  Accel (raw): [%d, %d, %d]\r\n", accel[0], accel[1],
                          accel[2]);
    comms::USB_CDC.printf("  Gyro  (raw): [%d, %d, %d]\r\n", gyro[0], gyro[1],
                          gyro[2]);
    comms::USB_CDC.printf("  Mag   (raw): [%d, %d, %d]\r\n", mag[0], mag[1],
                          mag[2]);
    comms::USB_CDC.printf("  Temp  (°C):  %.2f\r\n", temp_c);
    comms::USB_CDC.printf("\r\n");

    // Wait before next reading
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

int main() {
  // Initialize LED for status indication
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // Initialize USB CDC for communication
  comms::USB_CDC.init();

  // Configure IMU
  imu_config.addr_accel_gyro = ICM20948_ACCEL_GYRO_ADDR;
  imu_config.addr_mag = ICM20948_MAG_ADDR;
  imu_config.spi = spi0;
  imu_config.id = 1;

  // Create task for reading IMU data
  xTaskCreate(imu_task, "imu_task", 2048, NULL, 10, NULL);

  // Start FreeRTOS scheduler
  vTaskStartScheduler();

  return 0;
}
