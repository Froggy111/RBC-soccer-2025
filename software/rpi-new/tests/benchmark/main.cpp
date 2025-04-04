#include "position.hpp"
#include "processor.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

const int n_iter = 10000;

// Function to read the first frame from a video file
cv::Mat readFirstFrame(const std::string &video_path) {
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file " << video_path
                  << std::endl;
        return cv::Mat();
    }

    cv::Mat frame;
    for (int i = 0; i < 78; i++) {
        cap >> frame;
    }
    return frame;
}

// Function to clamp a value between min and max
float clamp(float value, float min_val, float max_val) {
    return std::max(min_val, std::min(value, max_val));
}

int main(int argc, char **argv) {
    // Set up parameters
    const int field_width        = 292;
    const int field_height       = 350;
    const std::string video_path = "480p.mp4";
    const int num_frames         = 360; // One frame per degree

    // Initialize processor
    camera::CamProcessor processor;

    // Read the first frame
    std::cout << "Reading first frame from " << video_path << "..."
              << std::endl;
    cv::Mat test_frame = readFirstFrame(video_path);
    if (test_frame.empty()) {
        std::cerr << "Failed to read the first frame. Exiting." << std::endl;
        return 1;
    }

    // generate random array of pos
    std::vector<Pos> positions;

    for (int i = 0; i < n_iter; i++) {
        positions.push_back(
			Pos(clamp(rand() % field_width, 0, field_width),
				clamp(rand() % field_height, 0, field_height),
				clamp(rand() % 360, 0, 360)));
	}
    
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n_iter; i++) {
        processor.calculate_loss(test_frame, positions[i]);
    }
    
	auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count();

    std::cout << "Duration: " << duration << " ms" << std::endl;
}