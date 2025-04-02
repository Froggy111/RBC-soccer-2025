#pragma once

#include <cstdint>
#include <cmath> // Include for round()
extern "C" {
#include "hardware/spi.h"
}

// Define register addresses if not already done
#ifndef REG_BANK_SEL
#define REG_BANK_SEL 0x7F
#endif
// Bank 2 Registers
#ifndef GYRO_SMPLRT_DIV
#define GYRO_SMPLRT_DIV 0x00
#endif
#ifndef GYRO_CONFIG_1
#define GYRO_CONFIG_1 0x01
#endif
#ifndef ACCEL_SMPLRT_DIV_1
#define ACCEL_SMPLRT_DIV_1 0x10
#endif
#ifndef ACCEL_SMPLRT_DIV_2
#define ACCEL_SMPLRT_DIV_2 0x11
#endif
#ifndef ACCEL_CONFIG
#define ACCEL_CONFIG 0x14
#endif
// Bank 0 Registers (add others as needed)
#ifndef PWR_MGMT_1
#define PWR_MGMT_1 0x06
#endif
#ifndef PWR_MGMT_2
#define PWR_MGMT_2 0x07
#endif
#ifndef WHO_AM_I_ICM20948
#define WHO_AM_I_ICM20948 0x00
#endif
#ifndef INT_PIN_CFG
#define INT_PIN_CFG 0x0F
#endif
#ifndef ACCEL_XOUT_H
#define ACCEL_XOUT_H 0x2D
#endif
#ifndef GYRO_XOUT_H
#define GYRO_XOUT_H 0x33
#endif
#ifndef TEMP_OUT_H
#define TEMP_OUT_H 0x39
#endif
// Magnetometer (AK09916 accessed via I2C Master on ICM or Bypass)
#ifndef AK09916_XOUT_L
#define AK09916_XOUT_L                                                         \
  0x11 // Example, confirm actual register if using bypass/master
#endif
#ifndef AK09916_CNTL2
#define AK09916_CNTL2 0x31 // Example, confirm actual register
#endif

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

// --- SPI Communication ---
void spi_configure(config_t *config);
void spi_write(config_t *config, const uint8_t *data, size_t len);
void spi_read(config_t *config, uint8_t addr, uint8_t *buffer,
              size_t len_buffer);
// Helper for switching banks
void set_bank(config_t *config, uint8_t bank);

// --- Initialization & Basic Config ---
int8_t init(config_t *config);
void set_mag_rate(config_t *config,
                  uint8_t mode); // Assuming bypass mode for direct mag access

// --- NEW: Configuration Functions ---
/**
     * @brief Configures Gyroscope ODR and DLPF.
     * @param config Pointer to config struct.
     * @param odr_hz Desired Output Data Rate in Hz. Rate is approximate, calculated from divider.
     *               Only applicable if DLPF is enabled (bypass_dlpf=false). Max ODR is 1125 Hz (divider=0).
     * @param dlpf_cfg Digital Low Pass Filter setting (0-7). See Table 16 in datasheet.
     *                 Set to 0 or 7 bypasses the filter internally for higher ODR.
     * @param bypass_dlpf Explicitly bypass DLPF (true) or enable DLPF (false). If true, dlpf_cfg is ignored,
     *                    ODR is fixed at internal rate (likely 1.1kHz or 9kHz, datasheet unclear), GYRO_SMPLRT_DIV is ignored.
     * @return 0 on success, -1 on failure (e.g., invalid ODR).
     */
int8_t set_gyro_config(config_t *config, float odr_hz, uint8_t dlpf_cfg,
                       bool bypass_dlpf);

/**
     * @brief Configures Accelerometer ODR, DLPF, and FSR.
     * @param config Pointer to config struct.
     * @param odr_hz Desired Output Data Rate in Hz. Rate is approximate, calculated from divider.
     *               Only applicable if DLPF is enabled (bypass_dlpf=false). Max ODR is 1125 Hz (divider=0).
     * @param dlpf_cfg Digital Low Pass Filter setting (0-7). See Table 18 in datasheet.
     *                 Set to 0 or 7 bypasses the filter internally for higher ODR.
     * @param fsr_g Full Scale Range setting (2, 4, 8, or 16 for +/- G).
     * @param bypass_dlpf Explicitly bypass DLPF (true) or enable DLPF (false). If true, dlpf_cfg is ignored,
     *                    ODR is fixed at internal rate (likely 1.1kHz or 4.5kHz, datasheet unclear), ACCEL_SMPLRT_DIV is ignored.
     * @return 0 on success, -1 on failure (e.g., invalid ODR or FSR).
     */
int8_t set_accel_config(config_t *config, float odr_hz, uint8_t dlpf_cfg,
                        uint8_t fsr_g, bool bypass_dlpf);

// --- Raw Data Reading ---
void read_raw_accel(config_t *config, int16_t accel[3]);
void read_raw_gyro(config_t *config, int16_t gyro[3]);
void read_raw_temp(config_t *config, int16_t *temp);
void read_raw_mag(config_t *config, int16_t mag[3]); // Assuming bypass mode

// --- Calibration ---
void cal_gyro(config_t *config, int16_t gyro_bias[3]);
void cal_accel(config_t *config, int16_t accel_bias[3]);
void cal_mag_simple(config_t *config,
                    int16_t mag_bias[3]); // Assuming bypass mode

// --- Calibrated Data Reading ---
void read_cal_gyro(config_t *config, int16_t gyro[3], int16_t bias[3]);
void read_cal_accel(config_t *config, int16_t accel[3], int16_t bias[3]);
void read_cal_mag(config_t *config, int16_t mag[3],
                  int16_t bias[3]); // Assuming bypass mode
void read_temp_c(config_t *config, float *temp);
} // namespace icm20948
