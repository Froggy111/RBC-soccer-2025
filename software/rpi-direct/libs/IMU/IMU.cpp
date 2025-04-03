#include "IMU.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "timer.hpp"
#include "types.hpp"

extern "C" {
#include <memory.h>
}

using namespace types;

namespace IMU {

// initialise all to 0
static RawIMUData current_raw_data = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
};

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
static CorrectedIMUData previous_corrected_data = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
};

static IntegratedData current_integrated_data = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
};

static Vec3f32 previous_position = {0, 0, 0};
static u64 previous_timestamp    = 0;

void init(void) {
    previous_timestamp = timer::us();
    comms::USB_CDC.registerTopPicoHandler(comms::RecvTopPicoIdentifiers::IMU,
                                          IMU_processor);
    return;
}
Vec3f32 position(void) { return current_integrated_data.position; }
Vec3f32 velocity(void) { return current_integrated_data.velocity; }
Vec3f32 accel(void) { return current_integrated_data.accel; }
Vec3f32 orientation(void) { return current_integrated_data.orientation; }
Vec3f32 angular_velocity(void) {
    return current_integrated_data.angular_velocity;
}

IntegratedData correct_absolute_value(const Vec3f32 &position, f32 heading,
                                      u64 timestamp,
                                      const IntegratedData &current_data) {
    IntegratedData new_data = current_data;
    // update current pos
    new_data.position = position * filter_weights::corrector_pos +
                        new_data.position * filter_weights::accel_pos;
    Vec3f32 absolute_velocity = (position - previous_position) /
                                ((float)(timestamp - previous_timestamp) *
                                 1e6); // timestamps are in us, we want seconds
    new_data.velocity = absolute_velocity * filter_weights::corrector_velocity +
                        new_data.velocity * filter_weights::accel_velocity;
    new_data.orientation.z = heading * filter_weights::corrector_pos +
                             new_data.orientation.z * filter_weights::gyro;
    previous_position  = new_data.position;
    previous_timestamp = timestamp;
    return new_data;
}

IntegratedData integrate(const ProcessedIMUData &processed_data,
                         const IntegratedData &current_data) {
    IntegratedData new_data = current_data;
    new_data.position += new_data.velocity * tick_time;
    new_data.velocity += new_data.accel * tick_time;
    new_data.accel = processed_data.accel;
    new_data.orientation += new_data.angular_velocity * tick_time;
    new_data.angular_velocity = processed_data.gyro;
    return new_data;
}

Vec3f32 correct_accel_bias(const Vec3f32 &accel_data,
                           const Vec3f32 &accel_bias) {
    return accel_data * accel_conversion - accel_bias;
}

Vec3f32 correct_gyro_bias(const Vec3f32 &gyro_data, const Vec3f32 &gyro_bias) {
    return gyro_data * gyro_conversion - gyro_bias;
}

ProcessedIMUData fuse_IMU_data(const CorrectedIMUData &data) {
    ProcessedIMUData processed_data = {{0, 0, 0}, {0, 0, 0}};
    // process out rotation components of accelerometer data
    processed_data.accel = (data.accel_1 + data.accel_2) / 2;
    // process gyro data
    // take average of gyro from both IMUs
    Vec3f32 gyro_average = (data.gyro_1 + data.gyro_2) / 2;
    // take centrifugal acceleration
    f32 centrifugal_accel = (fabs(data.accel_1.x) + fabs(data.accel_2.x) -
                             2 * fabs(processed_data.accel.x)) /
                            2;
    // centrifugal acceleration is (angular velocity)^2/r, thus:
    f32 accel_x_angular_velocity =
        fsqrt(centrifugal_accel * accel_distance_from_middle);

    // take angular acceleration
    f32 angular_acceleration = ((previous_corrected_data.accel_1.y -
                                 previous_corrected_data.accel_2.y) /
                                2) /
                               accel_distance_from_middle;
    // integrate
    f32 accel_y_angular_velocity =
        current_processed_data.gyro.z + angular_acceleration * tick_time;

    processed_data.gyro.x = gyro_average.x;
    processed_data.gyro.y = gyro_average.y;
    processed_data.gyro.z =
        gyro_average.z * rotation_weights::gyro +
        accel_x_angular_velocity * rotation_weights::accel_x +
        accel_y_angular_velocity * rotation_weights::accel_y;
    return processed_data;
}

void IMU_processor(const u8 *data, u16 data_len) {
    // read data
    memcpy(&current_raw_data, data, data_len);
    debug::debug("Received IMU data with length: %u", data_len);

    Vec3f32 axis_corrected_accel_1 = {
        (f32)current_raw_data.accel_1[accel_x_index],
        (f32)current_raw_data.accel_1[accel_y_index],
        (f32)current_raw_data.accel_1[accel_z_index],
    };
    debug::debug("Raw accel_1: [%f, %f, %f]", axis_corrected_accel_1.x,
                 axis_corrected_accel_1.y, axis_corrected_accel_1.z);

    Vec3f32 axis_corrected_accel_2 = {
        (f32)current_raw_data.accel_2[accel_x_index],
        (f32)current_raw_data.accel_2[accel_y_index],
        (f32)current_raw_data.accel_2[accel_z_index],
    };
    debug::debug("Raw accel_2: [%f, %f, %f]", axis_corrected_accel_2.x,
                 axis_corrected_accel_2.y, axis_corrected_accel_2.z);

    Vec3f32 axis_corrected_gyro_1 = {
        (f32)current_raw_data.gyro_1[gyro_x_index],
        (f32)current_raw_data.gyro_1[gyro_y_index],
        (f32)current_raw_data.gyro_1[gyro_z_index],
    };
    debug::debug("Raw gyro_1: [%f, %f, %f]", axis_corrected_gyro_1.x,
                 axis_corrected_gyro_1.y, axis_corrected_gyro_1.z);

    Vec3f32 axis_corrected_gyro_2 = {
        (f32)current_raw_data.gyro_2[gyro_x_index],
        (f32)current_raw_data.gyro_2[gyro_y_index],
        (f32)current_raw_data.gyro_2[gyro_z_index],
    };
    debug::debug("Raw gyro_2: [%f, %f, %f]", axis_corrected_gyro_2.x,
                 axis_corrected_gyro_2.y, axis_corrected_gyro_2.z);

    current_corrected_data.accel_1 = correct_accel_bias(
        axis_corrected_accel_1 * accel_1_signs, accel_1_bias);
    debug::debug(
        "Corrected accel_1: [%f, %f, %f]", current_corrected_data.accel_1.x,
        current_corrected_data.accel_1.y, current_corrected_data.accel_1.z);

    current_corrected_data.accel_2 = correct_accel_bias(
        axis_corrected_accel_2 * accel_2_signs, accel_2_bias);
    debug::debug(
        "Corrected accel_2: [%f, %f, %f]", current_corrected_data.accel_2.x,
        current_corrected_data.accel_2.y, current_corrected_data.accel_2.z);

    current_corrected_data.gyro_1 =
        correct_gyro_bias(axis_corrected_gyro_1 * gyro_1_signs, gyro_1_bias);
    debug::debug(
        "Corrected gyro_1: [%f, %f, %f]", current_corrected_data.gyro_1.x,
        current_corrected_data.gyro_1.y, current_corrected_data.gyro_1.z);

    current_corrected_data.gyro_2 =
        correct_gyro_bias(axis_corrected_gyro_2 * gyro_2_signs, gyro_2_bias);
    debug::debug(
        "Corrected gyro_2: [%f, %f, %f]", current_corrected_data.gyro_2.x,
        current_corrected_data.gyro_2.y, current_corrected_data.gyro_2.z);

    current_processed_data = fuse_IMU_data(current_corrected_data);
    debug::debug("Fused data: [accel: %f, %f, %f] [gyro: %f, %f, %f]",
                 current_processed_data.accel.x, current_processed_data.accel.y,
                 current_processed_data.accel.z, current_processed_data.gyro.x,
                 current_processed_data.gyro.y, current_processed_data.gyro.z);

    current_integrated_data =
        integrate(current_processed_data, current_integrated_data);
    debug::debug("Integrated data: [pos: %f, %f, %f] [orientation: %f, %f, %f]",
                 current_integrated_data.position.x,
                 current_integrated_data.position.y,
                 current_integrated_data.position.z,
                 current_integrated_data.orientation.x,
                 current_integrated_data.orientation.y,
                 current_integrated_data.orientation.z);
}
} // namespace IMU
