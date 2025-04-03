#pragma once

#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"

extern "C" {

#include <cmath>
}

namespace IMU {
// NOTE: ALL UNITS in mm, radians, seconds
// NOTE: weights for rotation calculation

namespace rotation_weights {
const types::f32 gyro_unnormalised = 3;
const types::f32 accel_x_unnormalised =
    1; // this is centrifugal acceleration to rotation
const types::f32 accel_y_unnormalised =
    1; // this is difference in y accel to angular acceleration, then integrated
const types::f32 total =
    gyro_unnormalised + accel_x_unnormalised + accel_y_unnormalised;
const types::f32 gyro    = gyro_unnormalised / total;
const types::f32 accel_x = accel_x_unnormalised / total;
const types::f32 accel_y = accel_y_unnormalised / total;
} // namespace rotation_weights

// NOTE: weights for filter / integration correction
namespace filter_weights {
const types::f32 corrector_pos_unnormalised = 8;
const types::f32 accel_pos_unnormalised     = 1;
const types::f32 total_pos =
    corrector_pos_unnormalised + accel_pos_unnormalised;
const types::f32 corrector_velocity_unnormalised = 8;
const types::f32 accel_velocity_unnormalised     = 1;
const types::f32 total_velocity =
    corrector_velocity_unnormalised + accel_velocity_unnormalised;
const types::f32 corrector_orientation_unnormalised = 2;
const types::f32 gyro_unnormalised                  = 1;
const types::f32 total_orientation =
    corrector_orientation_unnormalised + gyro_unnormalised;

const types::f32 corrector_pos = corrector_pos_unnormalised / total_pos;
const types::f32 accel_pos     = accel_pos_unnormalised / total_pos;
const types::f32 corrector_velocity =
    corrector_velocity_unnormalised / total_velocity;
const types::f32 accel_velocity = accel_velocity_unnormalised / total_velocity;
const types::f32 corrector_orientation =
    corrector_orientation_unnormalised / total_orientation;
const types::f32 gyro = gyro_unnormalised / total_orientation;
} // namespace filter_weights

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

// axes follow right hand convention
const types::u8 gyro_x_index = accel_x_index;
const types::u8 gyro_y_index = accel_y_index;
const types::u8 gyro_z_index =
    accel_z_index; // this is probably what you actually want (heading)

// these should be correct (IMU1 is on the left)
const types::Vec3f32 accel_1_signs = {-1, -1, 1};
const types::Vec3f32 accel_2_signs = {-accel_1_signs.x, -accel_1_signs.y,
                                      accel_1_signs.z};
// gyro follows right hand rule
const types::Vec3f32 gyro_1_signs = accel_1_signs;
const types::Vec3f32 gyro_2_signs = accel_2_signs;

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

struct IntegratedData {
    types::Vec3f32 position;
    types::Vec3f32 velocity;
    types::Vec3f32 accel;
    types::Vec3f32 orientation;
    types::Vec3f32 angular_velocity;
};

void init(void);
types::Vec3f32 position(void);
types::Vec3f32 velocity(void);
types::Vec3f32 accel(void);
types::Vec3f32 orientation(void);
types::Vec3f32 angular_velocity(void);

// timestamps are in microseconds, should be at the start of when the readings were taken (not processed yet!)
// heading should be anticlockwise
IntegratedData correct_absolute_value(const types::Vec3f32 &position,
                                      types::f32 heading, types::u64 timestamp,
                                      const IntegratedData &current_data);
IntegratedData integrate(const ProcessedIMUData &processed_data,
                         const IntegratedData &current_data);

types::Vec3f32 correct_accel_bias(const types::Vec3f32 &accel_data,
                                  const types::Vec3f32 &accel_bias);
types::Vec3f32 correct_gyro_bias(const types::Vec3f32 &gyro_data,
                                 const types::Vec3f32 &gyro_bias);
ProcessedIMUData fuse_IMU_data(const CorrectedIMUData &data);
void IMU_processor(const types::u8 *data, types::u16 data_len);
} // namespace IMU
