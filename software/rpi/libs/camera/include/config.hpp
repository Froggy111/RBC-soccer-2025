#pragma once

namespace camera {
enum Resolutions { RES_1232P = 0, RES_1080P = 1, RES_480P = 2 };

// ~ TO BE TUNED
// thresholds for white
const int COLOR_R_THRES = 180;
const int COLOR_G_THRES = 180;
const int COLOR_B_THRES = 180;

// regression
const int NUM_PARTICLES_PER_GENERATION = 5;
const int NUM_GENERATIONS = 20;
}