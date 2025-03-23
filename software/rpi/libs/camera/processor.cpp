#include "processor.hpp"
#include "config.hpp"
#include "field.hpp"
#include "position.hpp"
#include <cstdlib>
#include <tuple>

namespace camera {
std::tuple<std::pair<Pos, float>, std::pair<Pos, float>, std::pair<Pos, float>,
           std::pair<Pos, float>>
CamProcessor::get_points(const cv::Mat &frame) {
    // find minima from all 4 corners
    Pos top_left(0, 0);
    Pos top_right(frame.cols, 0);
    Pos bottom_left(0, frame.rows);
    Pos bottom_right(frame.cols, frame.rows);

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
        x = x * cos_theta - y * sin_theta;
        y = x * sin_theta + y * cos_theta;
        
        // * for every coordinate in the arr, check if it exists in the image
        if (x < 0 || x >= camera_image.cols || y < 0 ||
            y >= camera_image.rows) {
            continue;
        }

        // get that pixel in the camera image
        const cv::Vec3b* row = camera_image.ptr<cv::Vec3b>(y);
        cv::Vec3b pixel      = row[x];
        
        // check if the pixel is white
        if (!(pixel[0] > COLOR_R_THRES && pixel[1] > COLOR_G_THRES &&
              pixel[2] > COLOR_B_THRES)) {
            non_white += 1;
        }
        count += 1;
    }

    float loss = ((float)non_white) / (float)count;
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
            // randomize new guess properties by the loss
            Pos new_guess = best_guess;
            new_guess.x += (rand() % 100 - 50) * best_loss;
            new_guess.y += (rand() % 100 - 50) * best_loss;
            new_guess.heading +=
                (rand() % 1 - 0.5) *
                best_loss; // about 57 degrees of heading variance MAX

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