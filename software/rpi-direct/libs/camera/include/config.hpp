#pragma once

namespace camera {
enum Resolutions { RES_1232P = 0, RES_1080P = 1, RES_480P = 2 };

// ^ TO BE TUNED
// thresholds for white
const int COLOR_R_THRES = 160;
const int COLOR_G_THRES = 160;
const int COLOR_B_THRES = 160;

// image sizes
const int IMG_WIDTH = 480;
const int IMG_HEIGHT = 640;

// initial search
const int GRID_SEARCH_RADIUS = 60;
const int GRID_SEARCH_STEP = 2;
const int GRID_SEARCH_HEADING_STEP = 2;

// regression
const int PARTICLE_SEARCH_NUM = 25;
const int PARTICLE_SEARCH_GEN = 12;
const int PARTICLE_SEARCH_VAR = 3;

// gradient descent params
const int REGRESSION_MAX_ITERATIONS = 50;
const float REGRESSION_INITIAL_STEP_X = 5.0f;
const float REGRESSION_INITIAL_STEP_Y = 5.0f;
const float REGRESSION_INITIAL_STEP_HEADING = 0.05f;  // ~3 degrees
const float REGRESSION_STEP_DECAY = 0.7f;
const float REGRESSION_CONVERGENCE_THRESHOLD = 0.001f;
} // namespace camera