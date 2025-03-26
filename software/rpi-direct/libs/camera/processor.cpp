#include "processor.hpp"
#include "config.hpp"
#include "field.hpp"
#include "position.hpp"
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace camera {
int CamProcessor::current_frame = 0;
Pos CamProcessor::current_pos   = {0, 0, 0};

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
        if (final_x < 0 || final_x >= IMG_HEIGHT || final_y < 0 ||
            final_y >= IMG_WIDTH) {
            continue;
        }

        // printf("FINAL X: %d, FINAL Y: %d\n", final_x, final_y);
        const cv::Vec3b *row = camera_image.ptr<cv::Vec3b>(IMG_WIDTH - final_y);
        const cv::Vec3b &pixel = row[final_x];

        if (pixel[0] < COLOR_R_THRES || pixel[1] < COLOR_G_THRES ||
            pixel[2] < COLOR_B_THRES) {
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

std::pair<Pos, float> CamProcessor::find_minima_scatter(
    const cv::Mat &camera_image, Pos &initial_guess,
    int particles_per_generation, int particles_dispersal,
    int num_generations) {
    Pos best_guess  = initial_guess;
    float best_loss = calculate_loss(camera_image, best_guess);
    for (int i = 0; i < num_generations; i++) {
        Pos current_best_guess  = best_guess;
        float current_best_loss = best_loss;

        // disperse points x times
        for (int j = 0; j < particles_per_generation; j++) {
            Pos new_guess = best_guess;

            // randomize new guess properties, with the randomness proportional to the best_loss
            new_guess.x = (int)generate_random_number(
                new_guess.x, particles_dispersal, -FIELD_X_SIZE / 2,
                FIELD_X_SIZE / 2);
            new_guess.y = (int)generate_random_number(
                new_guess.y, particles_dispersal, -FIELD_Y_SIZE / 2,
                FIELD_Y_SIZE / 2);
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
CamProcessor::find_minima_smart_search(const cv::Mat &camera_image, Pos &center,
                                       int radius, int step, int heading_step) {
    Pos best_guess  = center;
    float best_loss = calculate_loss(camera_image, best_guess);

    // Search boundaries
    int x_min = -FIELD_X_SIZE / 2;
    int x_max = FIELD_X_SIZE / 2;
    int y_min = -FIELD_Y_SIZE / 2;
    int y_max = FIELD_Y_SIZE / 2;

    // Do the dense search first>
    for (int x = center.x - radius; x <= center.x + radius; x += step) {
        for (int y = center.y - radius; y <= center.y + radius; y += step) {
            if (x < x_min || x >= x_max || y < y_min || y >= y_max) {
                continue;
            }

            for (int heading = 0; heading < 360; heading += heading_step) {
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

std::pair<Pos, float>
CamProcessor::find_minima_regression(const cv::Mat &camera_image,
                                     Pos &initial_guess, float learning_rate,
                                     int num_iterations) {
    Pos current_pos    = initial_guess;
    float current_loss = calculate_loss(camera_image, current_pos);

    // Constants for computing numerical gradients - INCREASE DELTA
    const float delta = 5.0f; // Larger step size for position differentiation
    const float delta_angle = 0.05f; // Slightly increased step for angle
    const float momentum    = 0.6f;  // Reduced momentum to allow faster changes

    // Initialize previous gradients for momentum
    float prev_grad_x = 0.0f;
    float prev_grad_y = 0.0f;
    float prev_grad_h = 0.0f;

    // Learning rates with BALANCED scaling
    float lr_pos = learning_rate * 2.0f; // INCREASE position learning rate
    float lr_angle =
        learning_rate * 0.1f; // REDUCE heading learning rate relatively

    // Adaptive learning parameters
    float success_rate = 1.0f;

    // Track best position found
    Pos best_pos    = current_pos;
    float best_loss = current_loss;

    for (int i = 0; i < num_iterations; i++) {
        // Calculate numerical gradients using central difference method

        // X gradient
        Pos pos_x_plus = current_pos;
        pos_x_plus.x += delta;
        float loss_x_plus = calculate_loss(camera_image, pos_x_plus);

        Pos pos_x_minus = current_pos;
        pos_x_minus.x -= delta;
        float loss_x_minus = calculate_loss(camera_image, pos_x_minus);

        float grad_x = (loss_x_plus - loss_x_minus) / (2.0f * delta);

        // Y gradient
        Pos pos_y_plus = current_pos;
        pos_y_plus.y += delta;
        float loss_y_plus = calculate_loss(camera_image, pos_y_plus);

        Pos pos_y_minus = current_pos;
        pos_y_minus.y -= delta;
        float loss_y_minus = calculate_loss(camera_image, pos_y_minus);

        float grad_y = (loss_y_plus - loss_y_minus) / (2.0f * delta);

        // Heading gradient
        Pos pos_h_plus = current_pos;
        pos_h_plus.heading += delta_angle;
        float loss_h_plus = calculate_loss(camera_image, pos_h_plus);

        Pos pos_h_minus = current_pos;
        pos_h_minus.heading -= delta_angle;
        float loss_h_minus = calculate_loss(camera_image, pos_h_minus);

        float grad_h = (loss_h_plus - loss_h_minus) / (2.0f * delta_angle);

        // NORMALIZE GRADIENTS to make updates more balanced
        float max_grad = std::max(std::abs(grad_x),
                                  std::max(std::abs(grad_y), std::abs(grad_h)));
        if (max_grad > 0.001f) { // Avoid division by very small values
            grad_x /= max_grad;
            grad_y /= max_grad;
            grad_h /= max_grad;
        }

        // Apply momentum to gradients
        grad_x = grad_x * (1.0f - momentum) + prev_grad_x * momentum;
        grad_y = grad_y * (1.0f - momentum) + prev_grad_y * momentum;
        grad_h = grad_h * (1.0f - momentum) + prev_grad_h * momentum;

        // Store current gradients for next iteration
        prev_grad_x = grad_x;
        prev_grad_y = grad_y;
        prev_grad_h = grad_h;

        // Update position using gradient descent - AVOID int casting too early
        Pos new_pos = current_pos;
        // Store as floats first to accumulate small changes
        float new_x = new_pos.x - (grad_x * lr_pos * success_rate);
        float new_y = new_pos.y - (grad_y * lr_pos * success_rate);
        new_pos.x   = static_cast<int>(new_x); // Only convert to int at the end
        new_pos.y   = static_cast<int>(new_y);
        new_pos.heading -= grad_h * lr_angle * success_rate;

        // Enforce field boundaries
        new_pos.x =
            std::max(-FIELD_X_SIZE / 2, std::min(FIELD_X_SIZE / 2, new_pos.x));
        new_pos.y =
            std::max(-FIELD_Y_SIZE / 2, std::min(FIELD_Y_SIZE / 2, new_pos.y));

        // Normalize heading to [0, 2π)
        while (new_pos.heading < 0)
            new_pos.heading += 2.0f * M_PI;
        while (new_pos.heading >= 2.0f * M_PI)
            new_pos.heading -= 2.0f * M_PI;

        // Calculate new loss
        float new_loss = calculate_loss(camera_image, new_pos);

        // Update adaptive learning rate based on progress
        if (new_loss < current_loss) {
            // If loss improved, accept the new position and increase learning rate
            current_pos  = new_pos;
            current_loss = new_loss;
            success_rate =
                std::min(1.5f * success_rate, 2.0f); // More aggressive increase

            // Track best position found
            if (new_loss < best_loss) {
                best_pos  = new_pos;
                best_loss = new_loss;
            }

            // Early termination if loss is very low
            if (current_loss < 0.1f) {
                break;
            }
        } else {
            // If loss didn't improve, reduce learning rate but don't update position
            success_rate *= 0.7f; // More aggressive decrease

            // Break if learning rate becomes too small
            if (success_rate < 0.01f) {
                // Try resetting from best position with smaller learning rate
                if (i < num_iterations - 3) { // Only if we have iterations left
                    current_pos  = best_pos;
                    current_loss = best_loss;
                    success_rate = 0.5f; // Reset success rate to half
                    i++;                 // Count this as an extra iteration
                    continue;
                }
                break;
            }
        }
    }

    return std::make_pair(current_pos, current_loss);
}

void CamProcessor::process_frame(const cv::Mat &frame) {
    // * Initialize and do a grid search
    if (current_frame == 0) {
        current_pos = {0, 0, 0};
        std::pair<Pos, float> minima =
            find_minima_smart_search(frame, current_pos, 30, 2, 2);
        current_pos = minima.first;
        printf("Initial guess: x: %d, y: %d, heading: %f\n", current_pos.x,
               current_pos.y, current_pos.heading);
    }
    // * Do scatter
    else {
        auto points = find_minima_scatter(frame, current_pos);
        current_pos = points.first;
        printf("Scatter guess: x: %d, y: %d, heading: %f\n", current_pos.x,
               current_pos.y, current_pos.heading);
    }
    current_frame++;
}
} // namespace camera