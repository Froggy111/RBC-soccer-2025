#pragma once

#include "position.hpp"
#include <opencv2/opencv.hpp>

namespace camera {
class CamProcessor {
  public:
    CamProcessor(Pos initial_pos) { current_pos = initial_pos; }
    CamProcessor() = default;
    ~CamProcessor() = default;

    // * Constants
    static Pos current_pos;
    static int current_frame;

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
     * ^ Works well when we are close to the minima
     * 
     * @param camera_image 
     * @param initial_guess 
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float>
    find_minima_regress(const cv::Mat &camera_image,
      Pos &initial_guess, int particles_per_generation = 10, int particles_dispersal = 12,
      int num_generations = 3);

    /**
     * @brief Find the minima from an initial guess, using smart search
     * Aims to do grid search efficiently by searching middle first
     * ^ Works well for a good initial guess, and a small search radius
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