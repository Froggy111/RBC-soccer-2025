#pragma once

#include <opencv2/core/types.hpp>
namespace camera {
enum Resolutions { RES_1232P = 0, RES_1080P = 1, RES_480P = 2 };

// ^ TO BE TUNED
// ball detection
const int BALL_DETECTION_MIN_CONTOUR = 10;
const int BALL_DETECTION_MIN_BRIGHTNESS = 130;
const int BALL_DETECTION_HEADING_TOL = 15;
const cv::Scalar BALL_DETECTION_PURPLE_MASK_LOWER(130, 50, 100);
const cv::Scalar BALL_DETECTION_PURPLE_MASK_UPPER(175, 255, 255);
 
// thresholds for white
const int COLOR_R_THRES = 160;
const int COLOR_G_THRES = 160;
const int COLOR_B_THRES = 160;

// image sizes
const int IMG_WIDTH  = 480;
const int IMG_HEIGHT = 640;

// full search
const int FULL_SEARCH_STEP                  = 5;
const int FULL_SEARCH_HEADING_STEP          = 5;
const int FULL_SEARCH_INTERVAL              = 120;
const int FULL_SEARCH_WHITE_COUNT_THRESHOLD = 5;

// particle search
const int PARTICLE_SEARCH_NUM      = 150;
const int PARTICLE_SEARCH_GEN      = 1;
const int PARTICLE_SEARCH_VAR      = 0;
const int PARTICLE_SEARCH_VAR_HEAD = 40;

// regression
const int REGRESSION_MAX_ITERATIONS          = 10;
const float REGRESSION_INITIAL_STEP_X        = 3.0f;
const float REGRESSION_INITIAL_STEP_Y        = 3.0f;
const float REGRESSION_INITIAL_STEP_HEADING  = 0.05f; // ~3 degrees
const float REGRESSION_STEP_DECAY            = 0.7f;
const float REGRESSION_CONVERGENCE_THRESHOLD = 0.001f;
} // namespace camera