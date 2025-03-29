#include "config.hpp"
#include "position.hpp"
#include "processor.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

int main() {
    srand(4);

    const std::string input_path  = "./480p.mp4";
    const std::string output_path = "./points_data.csv";
    cv::VideoCapture cap(input_path);

    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open input video file: " << input_path
                  << std::endl;
        return -1;
    }


    camera::CamProcessor processor;
    cv::Mat frame;

    cap >> frame;
    processor.process_frame(frame);

    return 0;
}