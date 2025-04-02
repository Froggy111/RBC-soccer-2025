#pragma once

#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"

extern "C" {

#include <cmath>
}

namespace IMU {
// NOTE: ALL UNITS IN mm, radians, seconds
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

const types::f32 grav = 9.81 * 1000; // units in mm and seconds

// NOTE: bias means how much they initially read (when supposed to be zero)
const types::Vec3f32 accel_1_bias(0, 0, 9.81);    // calibrate
const types::Vec3f32 gyro_1_bias(0, 0, 0);        // calibrate also
const types::Vec3f32 accel_2_bias(0, 0, 9.81);    // calibrate
const types::Vec3f32 gyro_2_bias(0, 0, 0);        // calibrate also
const types::f32 accel_distance_from_middle = 25; // approximate, calibrate

const types::f32 tick_time = 1e-3; // 1ms per new accel and gyro val
const types::f32 FSR       = 8;    // 8 Gs max
const types::f32 accel_conversion =
    FSR * grav / std::numeric_limits<types::i16>::max();
const types::f32 gyro_conversion = M_PI / 180; // deg to rad

const types::u8 accel_x_index = 0; // x is forward/backward
const types::u8 accel_y_index = 1; // y is left/right
const types::u8 accel_z_index = 2; // z is up/down

// NOTE: weights for rotation calculation
namespace rotation_weights {

const types::f32 gyro_unnormalised    = 3;
const types::f32 accel_x_unnormalised = 0; // NOTE: unused, dont weigh for now
const types::f32 accel_y_unnormalised = 1;
const types::f32 total =
    gyro_unnormalised + accel_x_unnormalised + accel_y_unnormalised;
const types::f32 gyro    = gyro_unnormalised / total;
const types::f32 accel_x = accel_x_unnormalised / total;
const types::f32 accel_y = accel_y_unnormalised / total;

} // namespace rotation_weights

// signs for accel_1, accel_2 are inverse of this
const types::i8 accel_1_x_sign = 1;
const types::i8 accel_1_y_sign = 1;
const types::i8 accel_2_x_sign = -accel_1_x_sign;
const types::i8 accel_2_y_sign = -accel_1_y_sign;

// axes follow right hand convention
const types::u8 gyro_x_index = 0;
const types::u8 gyro_y_index = 1;
const types::u8 gyro_z_index =
    2; // this is probably what you actually want (heading)

void init(void);

types::Vec3f32 correct_accel_bias(const types::Vec3f32 &accel_data,
                                  const types::Vec3f32 &accel_bias);
types::Vec3f32 correct_gyro_bias(const types::Vec3f32 &gyro_data,
                                 const types::Vec3f32 &gyro_bias);
ProcessedIMUData fuse_IMU_data(const CorrectedIMUData &data);
void IMU_processor(const int *data, int data_len);
} // namespace IMU
