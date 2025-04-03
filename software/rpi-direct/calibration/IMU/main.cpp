#include "IMU.hpp"
#include "comms.hpp"
#include "debug.hpp"
#include "timer.hpp"
#include "types.hpp"
#include <memory.h>
#include <stdio.h>
#include <vector>

using namespace types;

namespace IMU_Calibration {

struct CalibrationData {
    std::vector<IMU::CorrectedIMUData> stationary_samples;
    std::vector<IMU::CorrectedIMUData> rotation_samples;
    bool collecting_data     = false;
    int current_step         = 0;
    int total_positions      = 6; // Number of stationary positions to sample
    int rotation_tests       = 3; // Number of rotation tests to perform
    int samples_per_position = 100;
    int samples_collected    = 0;
};

static CalibrationData calibration_data;
static IMU::CorrectedIMUData latest_corrected_data = {
    Vec3f32(0, 0, 0),
    Vec3f32(0, 0, 0),
    Vec3f32(0, 0, 0),
    Vec3f32(0, 0, 0),
};
static bool new_data_available = false;

// Function to process IMU data received from comms
void calibration_imu_processor(const u8 *data, u16 data_len) {
    IMU::RawIMUData raw_data;
    memcpy(&raw_data, data, data_len);

    // Convert raw data to corrected data (but without applying the biases we're trying to calculate)
    Vec3f32 axis_corrected_accel_1 = {
        (f32)raw_data.accel_1[IMU::accel_x_index],
        (f32)raw_data.accel_1[IMU::accel_y_index],
        (f32)raw_data.accel_1[IMU::accel_z_index],
    };

    Vec3f32 axis_corrected_accel_2 = {
        (f32)raw_data.accel_2[IMU::accel_x_index],
        (f32)raw_data.accel_2[IMU::accel_y_index],
        (f32)raw_data.accel_2[IMU::accel_z_index],
    };

    Vec3f32 axis_corrected_gyro_1 = {
        (f32)raw_data.gyro_1[IMU::gyro_x_index],
        (f32)raw_data.gyro_1[IMU::gyro_y_index],
        (f32)raw_data.gyro_1[IMU::gyro_z_index],
    };

    Vec3f32 axis_corrected_gyro_2 = {
        (f32)raw_data.gyro_2[IMU::gyro_x_index],
        (f32)raw_data.gyro_2[IMU::gyro_y_index],
        (f32)raw_data.gyro_2[IMU::gyro_z_index],
    };

    // Convert to proper units but don't apply the biases
    latest_corrected_data.accel_1 =
        axis_corrected_accel_1 * IMU::accel_1_signs * IMU::accel_conversion;
    latest_corrected_data.accel_2 =
        axis_corrected_accel_2 * IMU::accel_2_signs * IMU::accel_conversion;
    latest_corrected_data.gyro_1 =
        axis_corrected_gyro_1 * IMU::gyro_1_signs * IMU::gyro_conversion;
    latest_corrected_data.gyro_2 =
        axis_corrected_gyro_2 * IMU::gyro_2_signs * IMU::gyro_conversion;

    // Store data if we're collecting
    if (calibration_data.collecting_data) {
        if (calibration_data.current_step <= calibration_data.total_positions) {
            // Collecting stationary data
            calibration_data.stationary_samples.push_back(
                latest_corrected_data);
        } else {
            // Collecting rotation data
            calibration_data.rotation_samples.push_back(latest_corrected_data);
        }

        calibration_data.samples_collected++;
        if (calibration_data.samples_collected >=
            calibration_data.samples_per_position) {
            calibration_data.collecting_data = false;
            debug::debug("Data collection complete for step %d",
                         calibration_data.current_step);
        }
    }

    new_data_available = true;
}

// Calculate the average of a vector of IMU data
IMU::CorrectedIMUData
calculate_average(const std::vector<IMU::CorrectedIMUData> &samples) {
    IMU::CorrectedIMUData avg = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
    };

    if (samples.empty())
        return avg;

    for (const auto &sample : samples) {
        avg.accel_1 += sample.accel_1;
        avg.accel_2 += sample.accel_2;
        avg.gyro_1 += sample.gyro_1;
        avg.gyro_2 += sample.gyro_2;
    }

    avg.accel_1 = avg.accel_1 / samples.size();
    avg.accel_2 = avg.accel_2 / samples.size();
    avg.gyro_1  = avg.gyro_1 / samples.size();
    avg.gyro_2  = avg.gyro_2 / samples.size();

    return avg;
}

// Calculate the biases from the stationary readings
void calculate_imu_biases() {
    IMU::CorrectedIMUData avg =
        calculate_average(calibration_data.stationary_samples);

    // Calculate biases - for stationary positions, these should be the readings
    Vec3f32 accel_1_bias = avg.accel_1;
    Vec3f32 accel_2_bias = avg.accel_2;
    Vec3f32 gyro_1_bias  = avg.gyro_1;
    Vec3f32 gyro_2_bias  = avg.gyro_2;

    // Adjust for gravity on z axis
    accel_1_bias.z += IMU::grav; // Assuming +z is upward
    accel_2_bias.z += IMU::grav;

    printf("Calculated biases:\n");
    printf("accel_1_bias: (%f, %f, %f)\n", accel_1_bias.x, accel_1_bias.y,
           accel_1_bias.z);
    printf("accel_2_bias: (%f, %f, %f)\n", accel_2_bias.x, accel_2_bias.y,
           accel_2_bias.z);
    printf("gyro_1_bias: (%f, %f, %f)\n", gyro_1_bias.x, gyro_1_bias.y,
           gyro_1_bias.z);
    printf("gyro_2_bias: (%f, %f, %f)\n", gyro_2_bias.x, gyro_2_bias.y,
           gyro_2_bias.z);

    debug::debug("accel_1_bias(%f, %f, %f)", accel_1_bias.x, accel_1_bias.y,
                 accel_1_bias.z);
    debug::debug("gyro_1_bias(%f, %f, %f)", gyro_1_bias.x, gyro_1_bias.y,
                 gyro_1_bias.z);
    debug::debug("accel_2_bias(%f, %f, %f)", accel_2_bias.x, accel_2_bias.y,
                 accel_2_bias.z);
    debug::debug("gyro_2_bias(%f, %f, %f)", gyro_2_bias.x, gyro_2_bias.y,
                 gyro_2_bias.z);
}

// Calculate the distance from middle using rotation samples
void calculate_accel_distance_from_middle() {
    if (calibration_data.rotation_samples.empty()) {
        printf("No rotation samples collected. Cannot calculate distance.\n");
        return;
    }

    // Apply the biases calculated earlier to the rotation samples
    std::vector<IMU::ProcessedIMUData> processed_samples;

    for (const auto &sample : calibration_data.rotation_samples) {
        IMU::ProcessedIMUData processed = {Vec3f32(0, 0, 0), Vec3f32(0, 0, 0)};
        processed.accel                 = (sample.accel_1 + sample.accel_2) / 2;
        processed.gyro                  = (sample.gyro_1 + sample.gyro_2) / 2;
        processed_samples.push_back(processed);
    }

    // Find maximum angular velocity and corresponding centrifugal acceleration
    f32 max_angular_vel       = 0;
    f32 max_centrifugal_accel = 0;

    for (size_t i = 0; i < processed_samples.size(); i++) {
        f32 angular_vel_z = fabs(processed_samples[i].gyro.z);

        if (angular_vel_z > max_angular_vel) {
            max_angular_vel = angular_vel_z;

            // Calculate the centrifugal acceleration
            f32 accel_1_x =
                fabs(calibration_data.rotation_samples[i].accel_1.x);
            f32 accel_2_x =
                fabs(calibration_data.rotation_samples[i].accel_2.x);
            f32 center_accel_x = fabs(processed_samples[i].accel.x);

            max_centrifugal_accel =
                (accel_1_x + accel_2_x - 2 * center_accel_x) / 2;
        }
    }

    // Calculate distance: r = a/(ω²)
    f32 distance_from_middle = 0;
    if (max_angular_vel >
        0.1) { // Threshold to avoid division by very small numbers
        distance_from_middle =
            max_centrifugal_accel / (max_angular_vel * max_angular_vel);
    }

    printf("Estimated distance from middle: %f mm\n", distance_from_middle);
    debug::debug("accel_distance_from_middle = %f", distance_from_middle);
}

void run_calibration() {
    // Register our custom IMU processor
    comms::USB_CDC.registerTopPicoHandler(comms::RecvTopPicoIdentifiers::IMU,
                                          calibration_imu_processor);

    printf("IMU Calibration Program\n");
    printf("This program will guide you through calibrating your IMU "
           "sensors.\n\n");

    // Initialize calibration data
    calibration_data = CalibrationData();

    // Stationary positions calibration
    printf("We'll gather data from %d different stationary positions.\n",
           calibration_data.total_positions);

    for (int pos = 1; pos <= calibration_data.total_positions; pos++) {
        printf("\nPosition %d/%d:\n", pos, calibration_data.total_positions);
        printf(
            "Place the device on a flat surface in a different orientation.\n");
        printf("Press Enter when the device is stationary...");

        while (getchar() != '\n')
            ; // Wait for Enter key

        printf("Collecting data for position %d. Keep the device completely "
               "still...\n",
               pos);

        // Start collecting data
        calibration_data.collecting_data   = true;
        calibration_data.current_step      = pos;
        calibration_data.samples_collected = 0;

        // Wait for data collection to complete
        while (calibration_data.collecting_data) {
            // We could put a small delay here if needed
            if (new_data_available) {
                new_data_available = false;
                printf("Collected sample %d/%d\r",
                       calibration_data.samples_collected,
                       calibration_data.samples_per_position);
            }
        }

        printf("\nPosition %d data collection complete!\n", pos);
    }

    // Calculate the biases from stationary data
    calculate_imu_biases();

    printf("\nNow we'll calibrate the accelerometer distance from middle.\n");
    printf("We'll need to collect data while the device is rotating.\n");

    // Rotation tests for distance calculation
    for (int rot = 1; rot <= calibration_data.rotation_tests; rot++) {
        printf("\nRotation Test %d/%d:\n", rot,
               calibration_data.rotation_tests);
        printf("Hold the device and prepare to rotate it around its z-axis "
               "(yaw).\n");
        printf("Press Enter when ready to begin rotation...");

        while (getchar() != '\n')
            ; // Wait for Enter key

        printf("Begin rotating the device smoothly and consistently in a "
               "circle.\n");
        printf("Press Enter to start data collection...");

        while (getchar() != '\n')
            ; // Wait for Enter key

        // Start collecting rotation data
        calibration_data.collecting_data = true;
        calibration_data.current_step = calibration_data.total_positions + rot;
        calibration_data.samples_collected = 0;

        // Wait for data collection to complete
        while (calibration_data.collecting_data) {
            if (new_data_available) {
                new_data_available = false;
                printf("Collected sample %d/%d\r",
                       calibration_data.samples_collected,
                       calibration_data.samples_per_position);
            }
        }

        printf("\nRotation test %d data collection complete!\n", rot);
    }

    // Calculate the distance from middle
    calculate_accel_distance_from_middle();

    printf("\nCalibration complete!\n");
    printf(
        "Please update the IMU.hpp file with these new calibration values.\n");

    // Unregister our handler
    // Note: This assumes there's a way to unregister handlers, if not, this line should be removed
    // comms::USB_CDC.unregisterTopPicoHandler(comms::RecvTopPicoIdentifiers::IMU);
}

} // namespace IMU_Calibration

// Main function
int main() {
    IMU_Calibration::run_calibration();
    return 0;
}
