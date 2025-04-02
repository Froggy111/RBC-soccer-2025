#pragma once

#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"

namespace IMU {
// accel is fixed point of g's, and gyro is in degrees per second
struct RawIMUData {
    types::i16 accel_1[3], gyro_1[3], accel_2[3], gyro_2[3];
};

struct CorrectedIMUData { // corrected to units of mm and seconds
    types::Vec3f32 accel_1, gyro_1, accel_2, gyro_2;
};

struct ProcessedIMUData {
    types::Vec3f32 accel, gyro;
};

const types::Vec3f32 accel_1_bias(0, 0, 9.81); // calibrate
const types::Vec3f32 gyro_1_bias(0, 0, 0);     // calibrate also
const types::Vec3f32 accel_2_bias(0, 0, 9.81); // calibrate
const types::Vec3f32 gyro_2_bias(0, 0, 0);     // calibrate also

const types::f32 tick_time = 1e-3; // 1ms per new accel and gyro val
const types::f32 FSR       = 8;    // 8 Gs max

const types::u8 accel_x_index = 0;
const types::u8 accel_y_index = 1;
const types::u8 accel_z_index = 2;

// signs for accel_1, accel_2 are inverse of this
const types::i8 accel_1_x_sign = 1;
const types::i8 accel_1_y_sign = 1;
const types::i8 accel_2_x_sign = -accel_1_x_sign;
const types::i8 accel_2_y_sign = -accel_1_y_sign;

const types::u8 gyro_x_index = 0;
const types::u8 gyro_y_index = 1;
const types::u8 gyro_z_index = 2;

void init(void);

types::Vec3f32 correct_accel_bias(const types::Vec3f32 &accel_data,
                                  const types::Vec3f32 &accel_bias);
types::Vec3f32 correct_gyro_bias(const types::Vec3f32 &gyro_data,
                                 const types::Vec3f32 &gyro_bias);
ProcessedIMUData fuse_IMU_data(const CorrectedIMUData &data);
void IMU_processor(const int *data, int data_len);
} // namespace IMU
