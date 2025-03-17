#pragma once

#include <cstdint>
extern "C" {
#include "hardware/spi.h"
}

namespace icm20948 {
typedef struct config {
    uint8_t id;
    spi_inst_t *spi;
} config_t;

typedef struct data {
    // 0: x, 1: y, 2: z
    int16_t accel_raw[3];
    int16_t accel_bias[3];
    int16_t gyro_raw[3];
    int16_t gyro_bias[3];
    int16_t mag_raw[3];
    int16_t mag_bias[3];
    float temp_c;
} data_t;

void spi_configure(config_t *config);
void spi_write(config_t *config, const uint8_t * data, size_t len);
void spi_read(config_t *config, uint8_t addr, uint8_t * buffer, size_t len_buffer);

int8_t init(config_t *config);
void set_mag_rate(config_t *config, uint8_t mode);

void read_raw_accel(config_t *config, int16_t accel[3]);
void read_raw_gyro(config_t *config, int16_t gyro[3]);
void read_raw_temp(config_t *config, int16_t *temp);
void read_raw_mag(config_t *config, int16_t mag[3]);

void cal_gyro(config_t *config, int16_t gyro_bias[3]);
void cal_accel(config_t *config, int16_t accel_bias[3]);
void cal_mag_simple(config_t *config, int16_t mag_bias[3]);

void read_cal_gyro(config_t *config, int16_t gyro[3], int16_t bias[3]);
void read_cal_accel(config_t *config, int16_t accel[3], int16_t bias[3]);
void read_cal_mag(config_t *config, int16_t mag[3], int16_t bias[3]);
void read_temp_c(config_t *config, float* temp);
}