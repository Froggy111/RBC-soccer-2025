#pragma once

#include <opencv2/opencv.hpp>

class CamProcessor {
  public:
    CamProcessor()  = default;
    ~CamProcessor() = default;

    // Process a frame and perform any necessary operations
    static void processFrame(const cv::Mat &frame);
};