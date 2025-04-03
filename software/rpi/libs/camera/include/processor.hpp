#pragma once

#include "config.hpp"
#include "position.hpp"
#include <opencv2/opencv.hpp>

namespace camera {
class CamProcessor {
  private:
    // Helper method moved to private section
    static int generate_random_number(int mid, int variance, int min, int max);

    static int _frame_count;

  public:
    CamProcessor()  = default;
    ~CamProcessor() = default;
    static Pos current_pos;

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

    /**
     * @brief Same as calculate_loss, but using the chunks
     */
    static float calculate_loss_chunks(const cv::Mat &camera_image, Pos &guess);

    // * Functions to find the minima

    /**
     * @brief Find the minima from an initial guess, using a very simple particle dispersal search
     * ^ Works VERY well ONLY for a good initial guess
     * 
     * @param camera_image The camera image to process
     * @param initial_guess Initial position guess
     * @param num_particles Number of particles per generation
     * @param num_generations Number of generations to run
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float> find_minima_particle_search(
        const cv::Mat &camera_image, Pos &initial_guess,
        int num_particles                   = PARTICLE_SEARCH_NUM,
        int num_generations                 = PARTICLE_SEARCH_GEN,
        int variance_per_generation         = PARTICLE_SEARCH_VAR,
        int heading_varience_per_generation = PARTICLE_SEARCH_VAR_HEAD);
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
    find_minima_full_search(const cv::Mat &camera_image,
                            int step         = FULL_SEARCH_HEADING_STEP,
                            int heading_step = FULL_SEARCH_STEP);

    /**
     * @brief Find the minima using gradient descent regression
     * Efficiently refines position estimate by moving in the direction 
     * that most reduces the loss function
     * 
     * @param camera_image The camera image to process
     * @param initial_guess Initial position estimate
     * @param max_iterations Maximum number of iterations
     * @param initial_step_x Initial step size for x coordinate
     * @param initial_step_y Initial step size for y coordinate  
     * @param initial_step_heading Initial step size for heading
     * @param step_decay Factor to reduce step size when no improvement
     * @param convergence_threshold Threshold to determine convergence
     * @return std::pair<Pos, float> returns the position and the loss
     */
    static std::pair<Pos, float> find_minima_regression(
        const cv::Mat &camera_image, Pos &initial_guess,
        int max_iterations          = REGRESSION_MAX_ITERATIONS,
        float initial_step_x        = REGRESSION_INITIAL_STEP_X,
        float initial_step_y        = REGRESSION_INITIAL_STEP_Y,
        float initial_step_heading  = REGRESSION_INITIAL_STEP_HEADING,
        float step_decay            = REGRESSION_STEP_DECAY,
        float convergence_threshold = REGRESSION_CONVERGENCE_THRESHOLD);

    // Add to the CamProcessor class declaration
    static std::pair<Pos, float>
    find_minima_local_grid_search(const cv::Mat &camera_image, Pos &estimate,
                                  int x_variance, int y_variance,
                                  float heading_variance, int x_step,
                                  int y_step, float heading_step);
};
} // namespace camera