#include "ICM20948.hpp"
#include "comms.hpp"
#include "pinmap.hpp"
#include "registers.hpp"

extern "C" {
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <pico/time.h>
}

void icm20948::spi_configure(icm20948_config_t *config) {
  // init spi
  gpio_set_function((uint)pinmap::Pico::SPI0_SCLK, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MISO, GPIO_FUNC_SPI);
  gpio_set_function((uint)pinmap::Pico::SPI0_MOSI, GPIO_FUNC_SPI);
  spi_set_format(config->spi, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
}

void icm20948::spi_write(icm20948_config_t *config, uint8_t addr,
                         const uint8_t *data, size_t len) {
  spi_configure(config);

  comms::USB_CDC.printf("SPI Write - Sent: 0x%02X\n", addr);
  uint8_t buf[len + 1];
  buf[0] = addr & 0x7F;
  for (uint8_t i = 0; i < len; i++)
    buf[i + 1] = data[i];

  gpio_put(
      (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
      0);
  spi_write_blocking(config->spi, buf, len + 1);
  gpio_put(
      (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
      1);
  comms::USB_CDC.printf("SPI Write - Done\n");
  
  return;
}

void icm20948::spi_read(icm20948_config_t *config, uint8_t addr,
                        uint8_t *buffer, size_t len) {
  spi_configure(config);

  comms::USB_CDC.printf("SPI Read - Sent: 0x%02X\n", addr);
  uint8_t buf[1];
  buf[0] = addr | 0x80;

  gpio_put(
    (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
    0);
  spi_write_read_blocking(config->spi, buf, buffer, 1);
  gpio_put(
      (uint)(config->id == 1 ? pinmap::Pico::IMU1_NCS : pinmap::Pico::IMU2_NCS),
      1);
  comms::USB_CDC.printf("SPI Read - Done\n");
}

int8_t icm20948::icm20948_init(icm20948_config_t *config) {
  uint8_t reg[2], buf;

  comms::USB_CDC.printf("1\r\n");

  // init gpio pins
  if (config->id == 1) {
    gpio_init((uint)pinmap::Pico::IMU1_NCS);
    gpio_set_dir((uint)pinmap::Pico::IMU1_NCS, GPIO_OUT);
    gpio_put((uint)pinmap::Pico::IMU1_NCS, 1);
  } else {
    gpio_init((uint)pinmap::Pico::IMU2_NCS);
    gpio_set_dir((uint)pinmap::Pico::IMU2_NCS, GPIO_OUT);
    gpio_put((uint)pinmap::Pico::IMU2_NCS, 1);
  }

  comms::USB_CDC.printf("2\r\n");

  // wake up accel/gyro!
  // first write register then, write value
  reg[0] = PWR_MGMT_1;
  reg[1] = 0x00;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // switch to user bank to 0
  reg[0] = REG_BANK_SEL;
  reg[1] = 0x00;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // auto select clock source
  reg[0] = PWR_MGMT_1;
  reg[1] = 0x01;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // disable accel/gyro once
  reg[0] = PWR_MGMT_2;
  reg[1] = 0x3F;
  spi_write(config, config->addr_accel_gyro, reg, 2);
  sleep_ms(10);

  // enable accel/gyro (again)
  reg[0] = PWR_MGMT_2;
  reg[1] = 0x00;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // check if the accel/gyro could be accessed
  reg[0] = WHO_AM_I_ICM20948;
  spi_write(config, config->addr_accel_gyro, reg, 1);
  spi_read(config, config->addr_accel_gyro, &buf, 1);
#ifndef NDEBUG
  printf("AG. WHO_AM_I: 0x%X\n", buf);
#endif
  if (buf != 0xEA)
    return -1;

  // switch to user bank 2
  reg[0] = REG_BANK_SEL;
  reg[1] = 0x20;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // gyro config
  //
  // set full scale to +-
  // set noise bandwidth to
  // smaller bandwidth means lower noise level & slower max sample rate
  reg[0] = GYRO_CONFIG_1;
  reg[1] = 0x29;
  spi_write(config, config->addr_accel_gyro, reg, 2);
  //
  // set gyro output data rate to 100Hz
  // output_data_rate = 1.125kHz / (1 + GYRO_SMPLRT_DIV)
  // 1125 / 11 = 100
  reg[0] = GYRO_SMPLRT_DIV;
  reg[1] = 0x0A;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // accel config
  //
  // set full scale to +-2g
  // set noise bandwidth to 136Hz
  reg[0] = ACCEL_CONFIG;
  reg[1] = 0x11;
  spi_write(config, config->addr_accel_gyro, reg, 2);
  //
  // set accel output data rate to 100Hz
  // output_data_rate = 1.125kHz / (1 + ACCEL_SMPLRT_DIV)
  // 16 bits for ACCEL_SMPLRT_DIV
  reg[0] = ACCEL_SMPLRT_DIV_2;
  reg[1] = 0x0A;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // switch to user bank to 0
  reg[0] = REG_BANK_SEL;
  reg[1] = 0x00;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // wake up mag! (INT_PIN_CFG, BYPASS_EN = 1)
  reg[0] = INT_PIN_CFG;
  reg[1] = 0x02;
  spi_write(config, config->addr_accel_gyro, reg, 2);

  // check if the mag could be accessed
  reg[0] = 0x01;
  spi_write(config, config->addr_mag, reg, 1);
  spi_read(config, config->addr_mag, &buf, 1);
#ifndef NDEBUG
  printf("MAG. WHO_AM_I: 0x%X\n", buf);
#endif
  if (buf != 0x09)
    return -1;

  // config mag
  //
  // set mag mode, to measure continuously in 100Hz
  reg[0] = AK09916_CNTL2;
  reg[1] = 0x08;
  spi_write(config, config->addr_mag, reg, 2);

  return 0;
}

void icm20948::icm20948_set_mag_rate(icm20948_config_t *config, uint8_t mode) {
  // Single measurement              : mode = 0
  // Continuous measurement in  10 Hz: mode = 10
  // Continuous measurement in  20 Hz: mode = 20
  // Continuous measurement in  50 Hz: mode = 50
  // Continuous measurement in 100 Hz: mode = 100
  uint8_t reg[2];

  switch (mode) {
  case 0:
    // single shot
    // after measure, transits to power-down mode automatically
    reg[1] = 0x01;
    break;
  case 10:
    // 10Hz continuous
    reg[1] = 0x02;
    break;
  case 20:
    // 20Hz continuous
    reg[1] = 0x04;
    break;
  case 50:
    // 50Hz continuous
    reg[1] = 0x06;
    break;
  case 100:
    // 100Hz continuous
    reg[1] = 0x08;
    break;
  default:
#ifndef NDEBUG
    printf("error at icm20948_set_mag_mode: wrong mode %d\n", mode);
#endif
    return;
  }

  reg[0] = AK09916_CNTL2;
  spi_write(config, config->addr_mag, reg, 2);

  return;
}

void icm20948::icm20948_read_raw_accel(icm20948_config_t *config,
                                       int16_t accel[3]) {
  uint8_t buf[6];

  // accel: 2 bytes each axis
  uint8_t reg = ACCEL_XOUT_H;
  spi_write(config, config->addr_accel_gyro, &reg, 1);
  spi_read(config, config->addr_accel_gyro, buf, 6);

  for (uint8_t i = 0; i < 3; i++)
    accel[i] = (buf[i * 2] << 8 | buf[(i * 2) + 1]);

  return;
}

void icm20948::icm20948_read_raw_gyro(icm20948_config_t *config,
                                      int16_t gyro[3]) {
  uint8_t buf[6];

  // gyro: 2byte each axis
  uint8_t reg = GYRO_XOUT_H;
  spi_write(config, config->addr_accel_gyro, &reg, 1);
  spi_read(config, config->addr_accel_gyro, buf, 6);

  for (uint8_t i = 0; i < 3; i++)
    gyro[i] = (buf[i * 2] << 8 | buf[(i * 2) + 1]);

  return;
}

void icm20948::icm20948_read_raw_temp(icm20948_config_t *config,
                                      int16_t *temp) {
  uint8_t reg = TEMP_OUT_H, buf[2];
  spi_write(config, config->addr_accel_gyro, &reg, 1);
  spi_read(config, config->addr_accel_gyro, buf, 2);

  *temp = (buf[0] << 8 | buf[1]);

  return;
}

void icm20948::icm20948_read_raw_mag(icm20948_config_t *config,
                                     int16_t mag[3]) {
  uint8_t buf[8];

  uint8_t reg = AK09916_XOUT_L;
  spi_write(config, config->addr_mag, &reg, 1);
  spi_read(config, config->addr_mag, buf, 8);

  for (int i = 0; i < 3; i++)
    mag[i] = (buf[(i * 2) + 1] << 8 | buf[(i * 2)]);

#ifndef NDEBUG
  if ((buf[6] & 0x08) == 0x08)
    printf("mag: ST1: Sensor overflow\n");

  // printf below works only if we read 0x10
  //if ((buf[0] & 0x01) == 0x01) printf("mag: ST1: Data overrun\n");
  //if ((buf[0] & 0x02) != 0x02) printf("mag: ST1: Data is NOT ready\n");
#endif

  return;
}

void icm20948::icm20948_cal_gyro(icm20948_config_t *config,
                                 int16_t gyro_bias[3]) {
  int16_t buf[3] = {0};
  int32_t bias[3] = {0};

  for (uint8_t i = 0; i < 200; i++) {
    icm20948_read_raw_gyro(config, buf);
    for (uint8_t j = 0; j < 3; j++) {
      bias[j] += buf[j];
    }
    sleep_ms(25);
  }
  for (uint8_t i = 0; i < 3; i++)
    gyro_bias[i] = (int16_t)(bias[i] / 200);

  return;
}

void icm20948::icm20948_read_cal_gyro(icm20948_config_t *config,
                                      int16_t gyro[3], int16_t bias[3]) {
  icm20948_read_raw_gyro(config, gyro);
  for (uint8_t i = 0; i < 3; i++)
    gyro[i] -= bias[i];
  return;
}

void icm20948::icm20948_cal_accel(icm20948_config_t *config,
                                  int16_t accel_bias[3]) {
  int16_t buf[3] = {0};
  int32_t bias[3] = {0};

  for (uint8_t i = 0; i < 200; i++) {
    icm20948_read_raw_accel(config, buf);
    for (uint8_t j = 0; j < 3; j++) {
      if (j == 2)
        bias[j] += (buf[j] - 16384);
      else
        bias[j] += buf[j];
    }
    sleep_ms(25);
  }
  for (uint8_t i = 0; i < 3; i++)
    accel_bias[i] = (int16_t)(bias[i] / 200);
  return;
}

void icm20948::icm20948_read_cal_accel(icm20948_config_t *config,
                                       int16_t accel[3], int16_t bias[3]) {
  icm20948_read_raw_accel(config, accel);
  for (uint8_t i = 0; i < 3; i++)
    accel[i] -= bias[i];
  return;
}

void icm20948::icm20948_cal_mag_simple(icm20948_config_t *config,
                                       int16_t mag_bias[3]) {
  int16_t buf[3] = {0}, max[3] = {0}, min[3] = {0};
#ifndef NDEBUG
  printf("mag calibration: \nswing sensor for 360 deg\n");
#endif
  for (int i = 0; i < 1000; i++) {
    icm20948_read_raw_mag(config, buf);
    for (int j = 0; j < 3; j++) {
      if (buf[j] > max[j])
        max[j] = buf[j];
      if (buf[j] < min[j])
        min[j] = buf[j];
    }
    sleep_ms(10);
  }
  for (uint8_t i = 0; i < 3; i++)
    mag_bias[i] = (max[i] + min[i]) / 2;
  return;
}

void icm20948::icm20948_read_cal_mag(icm20948_config_t *config, int16_t mag[3],
                                     int16_t bias[3]) {
  icm20948_read_raw_mag(config, mag);
  for (uint8_t i = 0; i < 3; i++)
    mag[i] -= bias[i];
  return;
}

void icm20948::icm20948_read_temp_c(icm20948_config_t *config, float *temp) {
  int16_t tmp;
  icm20948_read_raw_temp(config, &tmp);
  // temp  = ((raw_value - ambient_temp) / speed_of_sound) + 21
  *temp = (((float)tmp - 21.0f) / 333.87) + 21.0f;
  return;
}