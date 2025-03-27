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

    // * Functions to find theminima

    /**
     * @brief Find the minima from an initial guess, using regression
     * ^ Works VERY well ONLY for a good initial guess
     * 
     * @param camera_image 
     * @param initial_guess 
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float>
    find_minima_regress(const cv::Mat &camera_image, Pos &initial_guess);

    /**
     * @brief Find the minima from an initial guess, using grid search
     * ^ This works well for small steps, but takes a long time
     * ^ Still slightly RNG
     * 
     * @param camera_image 
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float>
    find_minima_grid_search(const cv::Mat &camera_image);

    /**
     * @brief Find the minima from an initial guess, using smart search
     * Aims to do grid search efficiently by searching middle first
     * ^ Works quite well for most getting roughly where the bot is quickly
     * 
     * @param camera_image 
     * @param center 
     * @return std::pair<Pos, float> 
     */
    static std::pair<Pos, float>
    find_minima_smart_search(const cv::Mat &camera_image, Pos &center,
                             int RADIUS, int STEP, int HEADING_STEP);
                             
};
} // namespace camera