#pragma once

#include "position.hpp"
#include <opencv2/opencv.hpp>

namespace camera {
class CamProcessor {
  private:
    // Helper method moved to private section
    static int generate_random_number(int mid, int variance, int min, int max);

  public:
    CamProcessor()  = default;
    ~CamProcessor() = default;

    static std::tuple<std::pair<Pos, float>, std::pair<Pos, float>,
                      std::pair<Pos, float>, std::pair<Pos, float>>
    get_points(const cv::Mat &frame);

    /**
     * @brief Process a frame and perform any necessary operations
     */
    static void process_frame(const cv::Mat &frame);

    /**
     * @brief Calculate the loss based on the camera image and a guess position
     */
    static float calculate_loss(const cv::Mat &camera_image, Pos &guess);

    // * Functions to find the minima

    /**
     * @brief Find the minima from an initial guess, using regression
     * ^ Works VERY well ONLY for a good initial guess
     * 
     * @param camera_image The camera image to process
     * @param initial_guess Initial position guess
     * @param num_particles Number of particles per generation
     * @param num_generations Number of generations to run
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float>
    find_minima_regress(const cv::Mat &camera_image, Pos &initial_guess,
                        int num_particles = 25, int num_generations = 12,
                        int variance_per_generation = 3);

    /**
     * @brief Find the minima from an initial guess, using grid search
     * ^ This works well for small steps, but takes a long time
     * ^ Still slightly RNG
     * 
     * @param camera_image The camera image to process
     * @param grid_step Step size for x and y positions
     * @param grid_step_heading Step size for heading angles
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float>
    find_minima_grid_search(const cv::Mat &camera_image, int grid_step = 3,
                            int grid_step_heading = 3);

    /**
     * @brief Find the minima from an initial guess, using smart search
     * Aims to do grid search efficiently by searching middle first
     * ^ Works quite well for most getting roughly where the bot is quickly
     * 
     * @param camera_image The camera image to process
     * @param center Center position for the search
     * @param radius Search radius around the center
     * @param step Step size for x and y positions
     * @param heading_step Step size for heading angles
     * @return std::pair<Pos, float> 
     */
    static std::pair<Pos, float>
    find_minima_smart_search(const cv::Mat &camera_image, Pos &center,
                             int radius = 30, int step = 5,
                             int heading_step = 10);
};
} // namespace camera