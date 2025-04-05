#include "processor.hpp"
#include "IMU.hpp"
#include "config.hpp"
#include "debug.hpp"
#include "field.hpp"
#include "field_chunked.hpp"
#include "goalpost.hpp"
#include "include/ball.hpp"
#include "include/processor.hpp"
#include "position.hpp"
#include "types.hpp"
#include <cstdio>
#include <cstdlib>
#include <opencv2/core/cvdef.h>
#include <unistd.h>
#include <utility>

namespace camera {

int CamProcessor::generate_random_number(int mid, int variance, int min,
                                         int max) {
    // Generate a random number between mid - variance and mid + variance
    int random_number = mid - variance + (rand() % (int)(2 * variance + 1));
    if (random_number > max) {
        random_number = max - (random_number - max);
    } else if (random_number < min) {
        random_number = min + (min - random_number);
    }
    return random_number;
}

float CamProcessor::calculate_loss(const cv::Mat &camera_image, Pos &guess) {
    uint32_t count = 0, non_white = 0;

    // Fixed-point scaling factor (Q16.16 format)
    constexpr int FP_SHIFT = 16;
    constexpr int FP_ONE   = 1 << FP_SHIFT;

    // Convert angles to fixed point representation
    int32_t sin_theta_fp = static_cast<int32_t>(sin(guess.heading) * FP_ONE);
    int32_t cos_theta_fp = static_cast<int32_t>(cos(guess.heading) * FP_ONE);

    // Transform & rotate white line coords
    for (int i = 0; i < field::WHITE_LINES_LENGTH; i++) {
        int16_t x = field::WHITE_LINES[i][0];
        int16_t y = field::WHITE_LINES[i][1];

        // Transform to relative coordinates
        int32_t rel_x = x + guess.x;
        int32_t rel_y = y + guess.y;

        // Rotate using fixed-point arithmetic
        int32_t rotated_x_fp =
            (rel_x * cos_theta_fp - rel_y * sin_theta_fp) >> FP_SHIFT;
        int32_t rotated_y_fp =
            (rel_x * sin_theta_fp + rel_y * cos_theta_fp) >> FP_SHIFT;

        // Convert back to integer coordinate space with offset
        int32_t final_x = rotated_x_fp + IMG_WIDTH / 2;
        int32_t final_y = rotated_y_fp + IMG_HEIGHT / 2;

        // Check if the point is within IMAGE boundaries
        if (final_x < 0 || final_x >= IMG_WIDTH || final_y < 0 ||
            final_y >= IMG_HEIGHT) {
            continue;
        }

        // printf("FINAL X: %d, FINAL Y: %d\n", final_x, final_y);
        const cv::Vec3b *row   = camera_image.ptr<cv::Vec3b>(final_x);
        const cv::Vec3b &pixel = row[IMG_HEIGHT - final_y];

        if (!(pixel[0] > COLOR_R_THRES && pixel[1] > COLOR_G_THRES &&
              pixel[2] > COLOR_B_THRES)) {
            non_white++;
            // debug::log("White pixel at (%d, %d): (%d, %d, %d)",
            //            IMG_HEIGHT - final_y, final_x, pixel[0], pixel[1],
            //            pixel[2]);
        } else {
            // debug::log("Non-white pixel at (%d, %d): (%d, %d, %d)",
            //            IMG_HEIGHT - final_y, final_x, pixel[0], pixel[1],
            //            pixel[2]);
        }
        count++;
    }

    if (count == 0) {
        return 1.0f;
    }
    // printf("Count: %d, Non-white: %d\n", count, non_white);

    // Use integer division if possible, or at least avoid double casting
    float loss = static_cast<float>(non_white) / count;
    return loss;
}

float CamProcessor::calculate_loss_chunks(const cv::Mat &camera_image,
                                          Pos &guess) {
    uint32_t count = 0, non_white = 0;

    // Fixed-point scaling factor (Q16.16 format)
    constexpr int FP_SHIFT = 16;
    constexpr int FP_ONE   = 1 << FP_SHIFT;

    // Convert angles to fixed point representation
    int32_t sin_theta_fp = static_cast<int32_t>(sin(guess.heading) * FP_ONE);
    int32_t cos_theta_fp = static_cast<int32_t>(cos(guess.heading) * FP_ONE);

    // Transform & rotate white line coords
    for (int i = 0; i < field_chunked::WHITE_CHUNK_COUNT; i++) {
        int16_t x = field_chunked::WHITE_CHUNK_INDICES[i][0];
        int16_t y = field_chunked::WHITE_CHUNK_INDICES[i][1];

        // Transform to relative coordinates
        int32_t rel_x = x + guess.x / field_chunked::CHUNK_SIZE;
        int32_t rel_y = y + guess.y / field_chunked::CHUNK_SIZE;

        // Rotate using fixed-point arithmetic
        int32_t rotated_x_fp =
            (rel_x * cos_theta_fp - rel_y * sin_theta_fp) >> FP_SHIFT;
        int32_t rotated_y_fp =
            (rel_x * sin_theta_fp + rel_y * cos_theta_fp) >> FP_SHIFT;

        // Convert back to integer coordinate space with offset
        int32_t final_x =
            rotated_x_fp + (IMG_WIDTH / 2) / field_chunked::CHUNK_SIZE;
        int32_t final_y =
            rotated_y_fp + (IMG_HEIGHT / 2) / field_chunked::CHUNK_SIZE;

        // Check if the point is within IMAGE boundaries
        if (final_x < 0 || final_x >= IMG_WIDTH / field_chunked::CHUNK_SIZE ||
            final_y < 0 || final_y >= IMG_HEIGHT / field_chunked::CHUNK_SIZE) {
            continue;
        }

        const cv::Vec3b *row   = camera_image.ptr<cv::Vec3b>(final_x);
        const cv::Vec3b &pixel = row[IMG_HEIGHT - final_y];

        if (!(pixel[0] > COLOR_R_THRES && pixel[1] > COLOR_G_THRES &&
              pixel[2] > COLOR_B_THRES)) {
            non_white++;
            // debug::log("White pixel at (%d, %d): (%d, %d, %d)",
            //            IMG_HEIGHT - final_y, final_x, pixel[0], pixel[1],
            //            pixel[2]);
        } else {
            // debug::log("Non-white pixel at (%d, %d): (%d, %d, %d)",
            //            IMG_HEIGHT - final_y, final_x, pixel[0], pixel[1],
            //            pixel[2]);
        }
        count++;
    }

    if (count == 0) {
        return 1.0f;
    }

    // Use integer division if possible, or at least avoid double casting
    float loss = static_cast<float>(non_white) / count;
    return loss;
}

std::pair<Pos, float> CamProcessor::find_minima_particle_search(
    const cv::Mat &camera_image, Pos &initial_guess, int num_particles,
    int num_generations, int variance_per_generation,
    int heading_varience_per_generation) {
    Pos best_guess  = initial_guess;
    float best_loss = calculate_loss(camera_image, best_guess);

    for (int i = 0; i < num_generations; i++) {
        Pos current_best_guess  = best_guess;
        float current_best_loss = best_loss;

        // disperse points x times
        for (int j = 0; j < num_particles; j++) {
            Pos new_guess = best_guess;

            // randomize new guess properties, with the randomness proportional to the best_loss
            new_guess.x = (int)generate_random_number(
                new_guess.x, variance_per_generation, -field::FIELD_X_SIZE / 2,
                field::FIELD_X_SIZE / 2);
            new_guess.y = (int)generate_random_number(
                new_guess.y, variance_per_generation, -field::FIELD_Y_SIZE / 2,
                field::FIELD_Y_SIZE / 2);
            new_guess.heading =
                generate_random_number(
                    (int)(new_guess.heading * (float)180 / M_PI),
                    heading_varience_per_generation, 0, 360) *
                (float)M_PI / 180.0f;

            // calculate loss
            float new_loss = calculate_loss(camera_image, new_guess);
            if (new_loss < current_best_loss) {
                current_best_guess = new_guess;
                current_best_loss  = new_loss;
            }
        }

        // check if the new guess is better than the best guess
        if (current_best_loss < best_loss) {
            best_guess = current_best_guess;
            best_loss  = current_best_loss;
        }
    }

    return std::make_pair(best_guess, best_loss);
}

std::pair<Pos, float>
CamProcessor::find_minima_full_search(const cv::Mat &camera_image, int step,
                                      int heading_step) {
    // * shrink the image to speed up the search
    cv::Mat shrunk_img =
        cv::Mat(IMG_WIDTH / field_chunked::CHUNK_SIZE,
                IMG_HEIGHT / field_chunked::CHUNK_SIZE, CV_8UC1);

    int whiteness[IMG_WIDTH / field_chunked::CHUNK_SIZE]
                 [IMG_HEIGHT / field_chunked::CHUNK_SIZE] = {0};
    for (int i = 0; i < IMG_WIDTH / field_chunked::CHUNK_SIZE; i++) {
        for (int j = 0; j < IMG_HEIGHT / field_chunked::CHUNK_SIZE; j++) {
            for (int k = 0; k < field_chunked::CHUNK_SIZE; k++) {
                for (int l = 0; l < field_chunked::CHUNK_SIZE; l++) {
                    int corrected_x = i * field_chunked::CHUNK_SIZE + k;
                    int corrected_y =
                        IMG_HEIGHT - (j * field_chunked::CHUNK_SIZE + l);

                    if (corrected_x < 0 || corrected_x >= IMG_WIDTH ||
                        corrected_y < 0 || corrected_y >= IMG_HEIGHT) {
                        continue;
                    }

                    const cv::Vec3b *row =
                        camera_image.ptr<cv::Vec3b>(corrected_x);
                    const cv::Vec3b &pixel = row[corrected_y];
                    if (pixel[0] > COLOR_R_THRES && pixel[1] > COLOR_G_THRES &&
                        pixel[2] > COLOR_B_THRES) {
                        whiteness[i][j]++;
                    }
                }
            }
        }
    }

    // Create a new image with the whiteness values
    for (int i = 0; i < IMG_WIDTH / field_chunked::CHUNK_SIZE; i++) {
        for (int j = 0; j < IMG_HEIGHT / field_chunked::CHUNK_SIZE; j++) {
            // white if above threshold
            if (whiteness[i][j] > FULL_SEARCH_WHITE_COUNT_THRESHOLD) {
                shrunk_img.at<uint8_t>(i, j) = 255;
            } else {
                shrunk_img.at<uint8_t>(i, j) = 0;
            }
        }
    }

    Pos best_guess  = {0, 0, 0};
    float best_loss = 1;

    // Search boundaries
    int x_min = 0;
    int x_max = field::FIELD_Y_SIZE / 2;
    int y_min = 0; // TODO: CHANGE
    int y_max = field::FIELD_Y_SIZE / 2;

    for (int x = x_min; x <= x_max; x += step) {
        for (int y = y_min; y <= y_max; y += step) {
            for (int heading = 0; heading < 360; heading += heading_step) {
                Pos guess  = {x, y, heading * (float)M_PI / 180.0f};
                float loss = calculate_loss_chunks(shrunk_img, guess);

                if (loss < best_loss) {
                    best_guess = guess;
                    best_loss  = loss;
                }
            }
        }
    }

    return std::make_pair(best_guess, best_loss);
}

std::pair<Pos, float> CamProcessor::find_minima_regression(
    const cv::Mat &camera_image, Pos &initial_guess, int max_iterations,
    float initial_step_x, float initial_step_y, float initial_step_heading,
    float step_decay, float convergence_threshold) {

    Pos current_pos    = initial_guess;
    float current_loss = calculate_loss(camera_image, current_pos);

    float step_x       = initial_step_x;
    float step_y       = initial_step_y;
    float step_heading = initial_step_heading;

    // Keep track of iterations and improvement
    int iterations = 0;
    bool improved  = true;

    while (iterations < max_iterations &&
           (step_x > convergence_threshold || step_y > convergence_threshold ||
            step_heading > convergence_threshold) &&
           improved) {

        improved = false;
        iterations++;

        // Try different directions and pick the one with the lowest loss
        struct Movement {
            float dx, dy, dh;
            float loss;
            Pos pos;
        };

        // Define possible movements
        const int num_directions           = 6;
        Movement movements[num_directions] = {
            {step_x, 0, 0, 0, current_pos},       // +x
            {-step_x, 0, 0, 0, current_pos},      // -x
            {0, step_y, 0, 0, current_pos},       // +y
            {0, -step_y, 0, 0, current_pos},      // -y
            {0, 0, step_heading, 0, current_pos}, // +heading
            {0, 0, -step_heading, 0, current_pos} // -heading
        };

        // Try each movement and calculate loss
        for (int i = 0; i < num_directions; i++) {
            Pos test_pos = current_pos;
            test_pos.x += movements[i].dx;
            test_pos.y += movements[i].dy;
            test_pos.heading += movements[i].dh;

            // Wrap heading to [0, 2π]
            while (test_pos.heading < 0)
                test_pos.heading += 2 * M_PI;
            while (test_pos.heading >= 2 * M_PI)
                test_pos.heading -= 2 * M_PI;

            // Keep position within field bounds
            test_pos.x = std::max(std::min(test_pos.x, field::FIELD_X_SIZE / 2),
                                  -field::FIELD_X_SIZE / 2);
            test_pos.y = std::max(std::min(test_pos.y, field::FIELD_Y_SIZE / 2),
                                  -field::FIELD_Y_SIZE / 2);

            movements[i].loss = calculate_loss(camera_image, test_pos);
            movements[i].pos  = test_pos;
        }

        // Find the movement with the lowest loss
        int best_idx    = -1;
        float best_loss = current_loss;

        for (int i = 0; i < num_directions; i++) {
            if (movements[i].loss < best_loss) {
                best_loss = movements[i].loss;
                best_idx  = i;
            }
        }

        // If we found an improvement, update the position
        if (best_idx >= 0) {
            current_pos  = movements[best_idx].pos;
            current_loss = best_loss;
            improved     = true;
        } else {
            // No improvement found, reduce step sizes
            step_x *= step_decay;
            step_y *= step_decay;
            step_heading *= step_decay;
        }
    }

    return std::make_pair(current_pos, current_loss);
}

std::pair<Pos, float> CamProcessor::find_minima_local_grid_search(
    const cv::Mat &camera_image, Pos &estimate, int x_variance, int y_variance,
    float heading_variance, int x_step, int y_step, float heading_step) {

    Pos best_guess  = estimate;
    float best_loss = calculate_loss(camera_image, best_guess);

    // Calculate search boundaries
    int x_min   = estimate.x - x_variance;
    int x_max   = estimate.x + x_variance;
    int y_min   = estimate.y - y_variance;
    int y_max   = estimate.y + y_variance;
    float h_min = estimate.heading - heading_variance;
    float h_max = estimate.heading + heading_variance;

    // Constrain to field boundaries
    x_min = std::max(x_min, -field::FIELD_X_SIZE / 2);
    x_max = std::min(x_max, field::FIELD_X_SIZE / 2);
    y_min = std::max(y_min, -field::FIELD_Y_SIZE / 2);
    y_max = std::min(y_max, field::FIELD_Y_SIZE / 2);

    // Normalize heading range
    while (h_min < 0)
        h_min += 2 * M_PI;
    while (h_min >= 2 * M_PI)
        h_min -= 2 * M_PI;
    while (h_max < 0)
        h_max += 2 * M_PI;
    while (h_max >= 2 * M_PI)
        h_max -= 2 * M_PI;

    // Grid search
    for (int x = x_min; x <= x_max; x += x_step) {
        for (int y = y_min; y <= y_max; y += y_step) {
            // Handle wrap-around case for heading
            float h = h_min;
            while ((h_min < h_max && h <= h_max) ||
                   (h_min > h_max && (h <= h_max || h >= h_min))) {

                Pos guess  = {x, y, h};
                float loss = calculate_loss(camera_image, guess);

                if (loss < best_loss) {
                    best_guess = guess;
                    best_loss  = loss;
                }

                h += heading_step;
                if (h >= 2 * M_PI)
                    h -= 2 * M_PI;
            }
        }
    }

    return std::make_pair(best_guess, best_loss);
}

int CamProcessor::_frame_count = 0;
Pos CamProcessor::current_pos(-field::FIELD_X_SIZE / 2, 0, M_PI);

std::pair<GoalpostInfo, GoalpostInfo> CamProcessor::goalpost_info;
GoalpostDetector CamProcessor::goalpost_detector = GoalpostDetector();
BallDetector CamProcessor::ball_detector = BallDetector();

void CamProcessor::process_frame(const cv::Mat &frame) {
    goalpost_info = goalpost_detector.detectGoalposts(frame);
    ball_info = ball_detector.getBallInfo(frame, ball_heading);
    ball_distance = ball_detector.getDistanceToBall(ball_info);

    // types::Vec3f32 cur_pos_imu         = IMU::position();
    // types::Vec3f32 cur_orientation_imu = IMU::orientation();

    // debug::error("IMU: %f, %f, %f", cur_pos_imu.x, cur_pos_imu.y,
    //              cur_orientation_imu.z / M_PI * 180);

    // Pos estimated(current_pos.heading +
    //                   (cur_orientation_imu.z - last_orient_imu.z),
    //               current_pos.x + (cur_pos_imu.x - last_pos_imu.x) /
    //                                   field::FIELD_X_SIZE * 132 * 100,
    //               current_pos.y + (cur_pos_imu.y - last_pos_imu.y) /
    //                                   field::FIELD_Y_SIZE * 193 * 100);

    // // clamp
    // estimated.x = std::max(std::min(estimated.x, field::FIELD_X_SIZE / 2),
    //                        -field::FIELD_X_SIZE / 2);
    // estimated.y = std::max(std::min(estimated.y, field::FIELD_Y_SIZE / 2),
    //                        -field::FIELD_Y_SIZE / 2);
    // estimated.heading =
    //     std::max(std::min(estimated.heading, 2 * (float)M_PI), 0.0f);

    // debug::info("ESTIMATED: %d, %d, %f", estimated.x, estimated.y,
    //             estimated.heading / M_PI * 180);

    // last_pos_imu    = cur_pos_imu;
    // last_orient_imu = cur_orientation_imu;

    // auto res = find_minima_local_grid_search(
    //     frame, current_pos, 12, 12, 14 * M_PI / 180, 3, 3, 2 * M_PI / 180);

    // current_pos.x       = res.first.x;
    // current_pos.y       = res.first.y;
    // current_pos.heading = res.first.heading;
    // debug::warn("POSITION: %d, %d, %f (Loss: %f)", current_pos.x, current_pos.y,
    //             current_pos.heading / M_PI * 180, res.second);

    _frame_count += 1;
}

} // namespace camera