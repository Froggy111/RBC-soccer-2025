#include "processor.hpp"
#include "config.hpp"
#include "field.hpp"
#include "position.hpp"
#include <cstdio>
#include <cstdlib>
#include <utility>
#include "debug.hpp"

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
    return loss;
}

std::pair<Pos, float> CamProcessor::find_minima_particle_search(
    const cv::Mat &camera_image, Pos &initial_guess, int num_particles,
    int num_generations, int variance_per_generation) {
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
                new_guess.x, variance_per_generation, -FIELD_X_SIZE / 2,
                FIELD_X_SIZE / 2);
            new_guess.y = (int)generate_random_number(
                new_guess.y, variance_per_generation, -FIELD_Y_SIZE / 2,
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
    int x_min = -IMG_HEIGHT / 2;
    int x_max = IMG_HEIGHT / 2;
    int y_min = -IMG_WIDTH / 2;
    int y_max = IMG_WIDTH / 2;

    // Do the dense search first
    for (int x = center.x - radius; x <= center.x + radius; x += step) {
        for (int y = center.y - radius; y <= center.y + radius; y += step) {
            if (x < x_min || x >= x_max || y < y_min || y >= y_max) {
                continue;
            }

            for (int heading = 0; heading < 360; heading += heading_step) {
                Pos guess  = {x, y, heading * (float)M_PI / 180.0f};
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

std::pair<Pos, float> CamProcessor::find_minima_regression(
    const cv::Mat &camera_image, Pos &initial_guess,
    int max_iterations, float initial_step_x, float initial_step_y,
    float initial_step_heading, float step_decay, float convergence_threshold) {
    
    Pos current_pos = initial_guess;
    float current_loss = calculate_loss(camera_image, current_pos);
    
    float step_x = initial_step_x;
    float step_y = initial_step_y;
    float step_heading = initial_step_heading;
    
    // Keep track of iterations and improvement
    int iterations = 0;
    bool improved = true;
    
    while (iterations < max_iterations && (step_x > convergence_threshold || 
           step_y > convergence_threshold || 
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
        const int num_directions = 6;
        Movement movements[num_directions] = {
            {step_x, 0, 0, 0, current_pos},      // +x
            {-step_x, 0, 0, 0, current_pos},     // -x
            {0, step_y, 0, 0, current_pos},      // +y
            {0, -step_y, 0, 0, current_pos},     // -y
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
            while (test_pos.heading < 0) test_pos.heading += 2 * M_PI;
            while (test_pos.heading >= 2 * M_PI) test_pos.heading -= 2 * M_PI;
            
            // Keep position within field bounds
            test_pos.x = std::max(std::min(test_pos.x, FIELD_X_SIZE/2), -FIELD_X_SIZE/2);
            test_pos.y = std::max(std::min(test_pos.y, FIELD_Y_SIZE/2), -FIELD_Y_SIZE/2);
            
            movements[i].loss = calculate_loss(camera_image, test_pos);
            movements[i].pos = test_pos;
        }
        
        // Find the movement with the lowest loss
        int best_idx = -1;
        float best_loss = current_loss;
        
        for (int i = 0; i < num_directions; i++) {
            if (movements[i].loss < best_loss) {
                best_loss = movements[i].loss;
                best_idx = i;
            }
        }
        
        // If we found an improvement, update the position
        if (best_idx >= 0) {
            current_pos = movements[best_idx].pos;
            current_loss = best_loss;
            improved = true;
        } else {
            // No improvement found, reduce step sizes
            step_x *= step_decay;
            step_y *= step_decay;
            step_heading *= step_decay;
        }
    }
    
    return std::make_pair(current_pos, current_loss);
}

int CamProcessor::_frame_count = 0;
Pos CamProcessor::_current_pos = {0, 0, 0};

void CamProcessor::process_frame(const cv::Mat &frame) {
    debug::info("Frame %d", _frame_count);
    if (_frame_count == 0) {
        auto res = find_minima_smart_search(frame, _current_pos, GRID_SEARCH_RADIUS, GRID_SEARCH_STEP, GRID_SEARCH_HEADING_STEP);
        _current_pos = res.first;
        debug::log("Found position: (%d, %d, %f) with loss: %f",
                   _current_pos.x, _current_pos.y,
                   _current_pos.heading * (float)180 / M_PI, res.second);
    } else {
        auto res = find_minima_particle_search(frame, _current_pos, PARTICLE_SEARCH_NUM, PARTICLE_SEARCH_GEN, PARTICLE_SEARCH_VAR);
        _current_pos = res.first;
        debug::log("Found position: (%d, %d, %f) with loss: %f",
                   _current_pos.x, _current_pos.y,
                   _current_pos.heading * (float)180 / M_PI, res.second);
    }
    _frame_count += 1;
}

} // namespace camera