#pragma once

#include <opencv2/opencv.hpp>
#include "position.hpp"

namespace camera {
class CamProcessor {
  public:
    CamProcessor()  = default;
    ~CamProcessor() = default;

    // Process a frame and perform any necessary operations
    static void process_frame(const cv::Mat &frame);
    static void calculate_loss(const cv::Mat &frame, Pos &guess);
};
}