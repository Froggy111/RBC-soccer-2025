#pragma once

#include "position.hpp"
#include <opencv2/opencv.hpp>

namespace camera {
class CamProcessor {
  public:
    CamProcessor()  = default;
    ~CamProcessor() = default;

    static std::tuple<std::pair<Pos, float>, std::pair<Pos, float>,
                      std::pair<Pos, float>, std::pair<Pos, float>>
    get_points(const cv::Mat &frame);

    /**
     * @brief Process a frame and perform any necessary operations
     * 
     * @param frame 
     */
    static void process_frame(const cv::Mat &frame);

    /**
     * @brief Calculate the loss based on the camera image and a guess position
     * 
     * @param camera_image 
     * @param guess 
     * @return float 
     */
    static float calculate_loss(const cv::Mat &camera_image, Pos &guess);

    /**
     * @brief Find the local minima from an initial guess
     * 
     * @param camera_image 
     * @param initial_guess 
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float> find_minima(const cv::Mat &camera_image,
                                             Pos &initial_guess);
};
} // namespace camera