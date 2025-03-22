#include "processor.hpp"
#include "position.hpp"
#include "field.hpp"
#include "config.hpp"

namespace camera {
void CamProcessor::process_frame(const cv::Mat &frame) {}

uint32_t CamProcessor::calculate_loss(const cv::Mat &camera_image, Pos &guess) {
    std::tuple<int16_t, int16_t> white_lines_coords[WHITE_LINES_LENGTH];
    memcpy(white_lines_coords, WHITE_LINES, sizeof(white_lines_coords));

    // * cache the sin and cos values
    float sin_theta = sin(guess.heading);
    float cos_theta = cos(guess.heading);

    // * transform & rotate white line coords
    for (int i = 0; i < WHITE_LINES_LENGTH; i++) {
        int16_t x = std::get<0>(white_lines_coords[i]);
        int16_t y = std::get<1>(white_lines_coords[i]);

        // transform
        x -= guess.x;
        y -= guess.y;

        // rotate
        white_lines_coords[i] = std::make_tuple(
            static_cast<int16_t>(x * cos_theta - y * sin_theta),
            static_cast<int16_t>(x * sin_theta + y * cos_theta));
    }

    // * for every coordinate in the arr, check if it exists in the image
    uint32_t loss = 0;
    for (int i = 0; i < WHITE_LINES_LENGTH; i++) {
        int16_t x = std::get<0>(white_lines_coords[i]);
        int16_t y = std::get<1>(white_lines_coords[i]);

        if (x < 0 || x >= camera_image.cols || y < 0 ||
            y >= camera_image.rows) {
            continue;
        }

        // get that pixel in the camera image
        cv::Vec3b pixel = camera_image.at<cv::Vec3b>(y, x);
        // check if the pixel is white
        if (pixel[0] > COLOR_R_THRES && pixel[1] > COLOR_G_THRES &&
            pixel[2] > COLOR_B_THRES) {
            // * if it is white, add to the loss
            loss += 1;
        }
    }

    return loss;
}

} // namespace camera