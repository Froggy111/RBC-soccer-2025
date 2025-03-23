#include "processor.hpp"
#include "config.hpp"
#include "field.hpp"
#include "position.hpp"
#include <cstdio>
#include <cstdlib>
#include <tuple>

namespace camera {
std::tuple<std::pair<Pos, float>, std::pair<Pos, float>, std::pair<Pos, float>,
           std::pair<Pos, float>>
CamProcessor::get_points(const cv::Mat &frame) {
    // find minima from all 4 corners
    Pos top_left(5, 5);
    Pos top_right(frame.cols - 5, 5);
    Pos bottom_left(5, frame.rows - 5);
    Pos bottom_right(frame.cols - 5, frame.rows - 5);

    // find minima from all 4 corners
    std::pair<Pos, float> top_left_res     = find_minima(frame, top_left);
    std::pair<Pos, float> top_right_res    = find_minima(frame, top_right);
    std::pair<Pos, float> bottom_left_res  = find_minima(frame, bottom_left);
    std::pair<Pos, float> bottom_right_res = find_minima(frame, bottom_right);

    return std::make_tuple(top_left_res, top_right_res, bottom_left_res,
                           bottom_right_res);
}

float CamProcessor::calculate_loss(const cv::Mat &camera_image, Pos &guess) {
    uint32_t count = 0, non_white = 0;

    // * cache the sin and cos values
    float sin_theta = sin(guess.heading);
    float cos_theta = cos(guess.heading);

    // * transform & rotate white line coords
    for (int i = 0; i < WHITE_LINES_LENGTH; i++) {
        int16_t x = std::get<0>(WHITE_LINES[i]);
        int16_t y = std::get<1>(WHITE_LINES[i]);

        // transform
        x -= guess.x;
        y -= guess.y;

        // rotate
        float rotated_x = x * cos_theta - y * sin_theta;
        float rotated_y = x * sin_theta + y * cos_theta;
        x               = (int)rotated_x;
        y               = (int)rotated_y;

        // * for every coordinate in the arr, check if it exists in the image
        if (x < 0 || x >= camera_image.cols || y < 0 ||
            y >= camera_image.rows) {
            continue;
        }

        // get that pixel in the camera image
        const cv::Vec3b *row = camera_image.ptr<cv::Vec3b>(y);
        cv::Vec3b pixel      = row[x];

        // check if the pixel is white
        if (!(pixel[0] > COLOR_R_THRES && pixel[1] > COLOR_G_THRES &&
              pixel[2] > COLOR_B_THRES)) {
            non_white += 1;
        }
        count += 1;
    }

    // if (guess.heading != 0 && count == 0) {
    //     printf("Found none.\n");
    // }

    // if (guess.heading != 0 && count != 0) {
    //     printf("Found one.\n");
    // }

    if (count == 0) {
        return 1.0f;
    }

    float loss = (float)non_white / (float)count;
    return loss;
}

std::pair<Pos, float> CamProcessor::find_minima(const cv::Mat &camera_image,
                                                Pos &initial_guess) {
    Pos best_guess  = initial_guess;
    float best_loss = calculate_loss(camera_image, best_guess);
    for (int i = 0; i < NUM_GENERATIONS; i++) {
        Pos current_best_guess  = best_guess;
        float current_best_loss = best_loss;

        // disperse points x times
        for (int j = 0; j < NUM_PARTICLES_PER_GENERATION; j++) {
            // randomize new guess properties, with the randomness proportional to the best_loss
            // ensuring that the new guesses are within the bounds of the image (0, camera_image.cols), (0, camera_image.rows)
            Pos new_guess = best_guess;

            // TODO: Ensure that the new guess is within the bounds of the image
            new_guess.x += (rand() % (int)(best_loss * 2 * camera_image.cols)) -
                           (best_loss * camera_image.cols);
            new_guess.y += (rand() % (int)(best_loss * 2 * camera_image.rows)) -
                           (best_loss * camera_image.rows);
            new_guess.heading +=
                (rand() % (int)(best_loss * M_PI)) - (best_loss * M_PI / 2);
            
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
} // namespace camera