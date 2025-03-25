#include "processor.hpp"
#include "config.hpp"
#include "field.hpp"
#include "position.hpp"
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace camera {
inline int generate_random_number(int mid, int variance, int min, int max) {
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
    for (int i = 0; i < WHITE_LINES_LENGTH; i++) {
        int16_t x = WHITE_LINES[i][0];
        int16_t y = WHITE_LINES[i][1];

        // Transform to relative coordinates
        int32_t rel_x = x + guess.x / GRID_SIZE;
        int32_t rel_y = y + guess.y / GRID_SIZE;

        // Rotate using fixed-point arithmetic
        int32_t rotated_x_fp =
            (rel_x * cos_theta_fp - rel_y * sin_theta_fp) >> FP_SHIFT;
        int32_t rotated_y_fp =
            (rel_x * sin_theta_fp + rel_y * cos_theta_fp) >> FP_SHIFT;

        // Convert back to integer coordinate space with offset
        int32_t final_x = rotated_x_fp + IMG_HEIGHT / 2;
        int32_t final_y = rotated_y_fp + IMG_WIDTH / 2;

        // Check if the point is within IMAGE boundaries
        if (final_x < 0 || final_x >= IMG_HEIGHT || final_y < 0 || final_y >= IMG_WIDTH) {
            continue;
        }

        // printf("FINAL X: %d, FINAL Y: %d\n", final_x, final_y);
        const cv::Vec3b *row = camera_image.ptr<cv::Vec3b>(IMG_WIDTH - final_y);
        const cv::Vec3b &pixel = row[final_x];

        if (pixel[0] < COLOR_R_THRES || pixel[1] < COLOR_G_THRES || pixel[2] < COLOR_B_THRES) {
            non_white++;
        }
        count++;
    }

    if (count == 0) {
        return 1.0f;
    }
    // printf("Count: %d, Non-white: %d\n", count, non_white);

    // Use integer division if possible, or at least avoid double casting
    float loss = static_cast<float>(non_white) / count;
    return loss * loss;
}

std::pair<Pos, float>
CamProcessor::find_minima_regress(const cv::Mat &camera_image,
                                  Pos &initial_guess) {
    // ? CONSTANTS
    const int NUM_PARTICLES_PER_GENERATION = 100;
    const int NUM_GENERATIONS              = 5;

    Pos best_guess  = initial_guess;
    float best_loss = calculate_loss(camera_image, best_guess);
    for (int i = 0; i < NUM_GENERATIONS; i++) {
        Pos current_best_guess  = best_guess;
        float current_best_loss = best_loss;

        // disperse points x times
        for (int j = 0; j < NUM_PARTICLES_PER_GENERATION; j++) {
            Pos new_guess = best_guess;

            // randomize new guess properties, with the randomness proportional to the best_loss
            new_guess.x =
                (int)generate_random_number(new_guess.x, 3, 0, FIELD_X_SIZE);
            new_guess.y =
                (int)generate_random_number(new_guess.y, 3, 0, FIELD_Y_SIZE);
            new_guess.heading =
                generate_random_number(
                    (int)(new_guess.heading * (float)180 / M_PI), 10, 0, 360) *
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
CamProcessor::find_minima_grid_search(const cv::Mat &camera_image) {
    // ? CONSTANTS
    const int GRID_STEP         = 3;
    const int GRID_STEP_HEADING = 3;

    Pos best_guess  = {0, 0, 0};
    float best_loss = 1.0f;

    // Perform grid search
    for (int x = 0; x < FIELD_X_SIZE; x += GRID_STEP) {
        for (int y = 0; y < FIELD_Y_SIZE; y += GRID_STEP) {
            for (int heading = 0; heading < 360; heading += GRID_STEP_HEADING) {
                Pos guess  = {x, y, heading * (float)M_PI / 180.0f};
                float loss = calculate_loss(camera_image, guess);

                // Check if the new guess is better than the best guess
                if (loss < best_loss) {
                    best_guess = guess;
                    best_loss  = loss;
                }

                if (best_loss < 0.4f) {
                    return std::make_pair(best_guess, best_loss);
                }
            }
        }
    }

    return std::make_pair(best_guess, best_loss);
}

std::pair<Pos, float>
CamProcessor::find_minima_smart_search(const cv::Mat &camera_image, Pos &center,
                                       int RADIUS, int STEP, int HEADING_STEP) {
    Pos best_guess  = center;
    float best_loss = calculate_loss(camera_image, best_guess);

    // Search boundaries
    int x_min = -IMG_WIDTH / 2;
    int x_max = IMG_WIDTH / 2;
    int y_min = -IMG_HEIGHT / 2;
    int y_max = IMG_HEIGHT / 2;

    // Do the dense search first>
    for (int x = center.x - RADIUS; x <= center.x + RADIUS; x += STEP) {
        for (int y = center.y - RADIUS; y <= center.y + RADIUS; y += STEP) {
            if (x < x_min || x >= x_max || y < y_min || y >= y_max) {
                continue;
            }

            for (int heading = 0; heading < 360; heading += HEADING_STEP) {
                Pos guess = {x, y, heading * (float)M_PI / 180.0f};
                // printf("x: %d, y: %d, heading: %d\n", x, y, heading);
                float loss = calculate_loss(camera_image, guess);

                // Check if the new guess is better than the best guess
                if (loss < best_loss) {
                    best_guess = guess;
                    best_loss  = loss;
                }
            }
        }
    }
    // If the loss is already low, return the best guess
    if (best_loss < 0.4f) {
        return std::make_pair(best_guess, best_loss);
    }

    return std::make_pair(best_guess, best_loss);
}
} // namespace camera