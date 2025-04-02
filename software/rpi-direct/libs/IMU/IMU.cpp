#include "IMU.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"

using namespace types;

namespace IMU {
static RawIMUData current_raw_data;
static CorrectedIMUData current_corrected_data = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
};
static ProcessedIMUData current_processed_data = {
    {0, 0, 0},
    {0, 0, 0},
};

Vec3f32 correct_accel_bias(const Vec3f32 &accel_data,
                           const Vec3f32 &accel_bias) {
    Vec3f32 corrected_accel = {0, 0, 0};
    corrected_accel.x       = accel_data.x * accel_conversion - accel_bias.x;
    corrected_accel.y       = accel_data.y * accel_conversion - accel_bias.y;
    corrected_accel.z       = accel_data.z * accel_conversion - accel_bias.z;
    return corrected_accel;
}

Vec3f32 correct_gyro_bias(const Vec3f32 &gyro_data, const Vec3f32 &gyro_bias) {
    Vec3f32 corrected_gyro = {0, 0, 0};
    corrected_gyro.x       = gyro_data.x * gyro_conversion - gyro_bias.x;
    corrected_gyro.y       = gyro_data.y * gyro_conversion - gyro_bias.y;
    corrected_gyro.z       = gyro_data.z * gyro_conversion - gyro_bias.z;
    return corrected_gyro;
}

ProcessedIMUData fuse_IMU_data(const CorrectedIMUData &data) {
    ProcessedIMUData processed_data = {{0, 0, 0}, {0, 0, 0}};
    // process out rotation components of accelerometer data
    processed_data.accel.x = (data.accel_1.x + data.accel_2.x) / 2;
    processed_data.accel.y = (data.accel_1.y + data.accel_2.y) / 2;
    processed_data.accel.z = (data.accel_1.z + data.accel_2.z) / 2;
    // process gyro data
    // take average of gyro from both IMUs
    Vec3f32 gyro_average = {(data.gyro_1.x + data.gyro_2.x) / 2,
                            (data.gyro_1.y + data.gyro_2.y) / 2,
                            (data.gyro_1.z + data.gyro_2.z) / 2};
    // take centrifugal acceleration
    f32 centrifugal_accel = (fabs(data.accel_1.y) + fabs(data.accel_2.y) -
                             2 * fabs(processed_data.accel.y)) /
                            2;
    // centrifugal acceleration is (angular velocity)^2/r, thus:
    f32 accel_y_angular_velocity =
        fsqrt(centrifugal_accel * accel_distance_from_middle);

    // add from fused accel_x maybe? but needs persistent velocity data

    processed_data.gyro.x = gyro_average.x;
    processed_data.gyro.y = gyro_average.y;
    processed_data.gyro.z =
        gyro_average.z * rotation_weights::gyro +
        accel_y_angular_velocity * rotation_weights::accel_y;

    return processed_data;
}

void IMU_processor(const int *data, int data_len) {
    // read data
    memcpy(&current_raw_data, data, data_len);
    current_corrected_data.accel_1 =
        correct_accel_bias(Vec3f32(current_raw_data.accel_1[accel_x_index],
                                   current_raw_data.accel_1[accel_y_index],
                                   current_raw_data.accel_1[accel_z_index]),
                           accel_1_bias);
    current_corrected_data.accel_2 =
        correct_accel_bias(Vec3f32(current_raw_data.accel_1[accel_x_index],
                                   current_raw_data.accel_1[accel_y_index],
                                   current_raw_data.accel_1[accel_z_index]),
                           accel_2_bias);
    current_corrected_data.gyro_1 =
        correct_gyro_bias(Vec3f32(current_raw_data.gyro_1[accel_x_index],
                                  current_raw_data.gyro_1[accel_y_index],
                                  current_raw_data.gyro_1[accel_z_index]),
                          gyro_1_bias);
    current_corrected_data.gyro_2 =
        correct_gyro_bias(Vec3f32(current_raw_data.gyro_2[accel_x_index],
                                  current_raw_data.gyro_2[accel_y_index],
                                  current_raw_data.gyro_2[accel_z_index]),
                          gyro_2_bias);
    current_processed_data = fuse_IMU_data(current_corrected_data);
}
} // namespace IMU
