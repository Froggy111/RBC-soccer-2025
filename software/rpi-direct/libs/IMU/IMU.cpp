#include "IMU.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"

using namespace types;

namespace IMU {
static RawIMUData current_raw_data;
static CorrectedIMUData current_corrected_data = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

Vec3f32 correct_accel_bias(const Vec3f32 &accel_data,
                           const Vec3f32 &accel_bias);
Vec3f32 correct_gyro_bias(const Vec3f32 &gyro_data, const Vec3f32 &gyro_bias);
ProcessedIMUData fuse_IMU_data(const CorrectedIMUData &data);

void IMU_processor(const int *data, int data_len) {
    // read data
    memcpy(&current_raw_data, data, data_len);
    current_corrected_data.accel_1 =
        correct_accel_bias(Vec3f32(current_raw_data.accel_1), accel_1_bias);
    current_corrected_data.accel_2 =
        correct_accel_bias(Vec3f32(current_raw_data.accel_2), accel_2_bias);
    current_corrected_data.gyro_1 =
        correct_gyro_bias(Vec3f32(current_raw_data.gyro_1), gyro_1_bias);
    current_corrected_data.gyro_2 =
        correct_gyro_bias(Vec3f32(current_raw_data.gyro_2), gyro_2_bias);
}
} // namespace IMU
