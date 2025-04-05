#include "goalpost.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <unistd.h>

int main(int argc, char **argv) {
    // Default video path with option to override via command line
    std::string videoPath = "vid4.mp4";

    // Check if a video path was provided as an argument
    if (argc > 1) {
        videoPath = argv[1];
        std::cout << "Using video: " << videoPath << std::endl;
    } else {
        std::cout << "No video path provided. Using default: " << videoPath
                  << std::endl;
    }

    // Check if file exists
    std::ifstream file(videoPath);
    if (!file.good()) {
        std::cerr << "Error: Input video file not found: " << videoPath
                  << std::endl;
        return -1;
    }

    // Open video
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video: " << videoPath << std::endl;
        return -1;
    }

    // Create detector with debug mode
    GoalpostDetector detector(true);

    // Optional: Set custom center point
    // detector.setCenterPoint(cv::Point(640/2 - 5, 480/2 + 30));

    // Process video frames
    while (true) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            break;
        }

        // Start timing
        auto startTime = std::chrono::high_resolution_clock::now();

        // Detect goalposts and get output frame
        cv::Mat outputFrame;
        auto [blueGoalInfo, yellowGoalInfo] =
            detector.detectGoalposts(frame, &outputFrame);

        // End timing
        auto endTime  = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        // Print detection results
        std::cout << "Time taken for frame: " << duration.count() << " ms"
                  << std::endl;

        if (blueGoalInfo.detected) {
            std::cout << "Blue goal detected at (" << blueGoalInfo.midpoint.x
                      << ", " << blueGoalInfo.midpoint.y << ")" << std::endl;
            std::cout << "Blue goal angle: "
                      << (blueGoalInfo.angle * 180 / M_PI) << " degrees"
                      << std::endl;
            std::cout << "Distance to Blue Goal: " << blueGoalInfo.distance
                      << std::endl;
        }

        if (yellowGoalInfo.detected) {
            std::cout << "Yellow goal detected at ("
                      << yellowGoalInfo.midpoint.x << ", "
                      << yellowGoalInfo.midpoint.y << ")" << std::endl;
            std::cout << "Yellow goal angle: "
                      << (yellowGoalInfo.angle * 180 / M_PI) << " degrees"
                      << std::endl;
            std::cout << "Distance to Yellow Goal: " << yellowGoalInfo.distance
                      << std::endl;
        }

        // Display result
        cv::imshow("Goal Detection", outputFrame);

        // Break loop if 'q' pressed
        if (cv::waitKey(25) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}