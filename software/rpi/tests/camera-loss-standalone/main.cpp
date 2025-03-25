#include "processor.hpp"
#include "position.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

// Helper function to convert degrees to radians
inline float deg_to_rad(float degrees) {
    return degrees * M_PI / 180.0f;
}

int main(int argc, char* argv[]) {
    // Check if arguments are provided
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <x_position> <y_position> <heading_degrees>" << std::endl;
        std::cout << "Example: " << argv[0] << " 100 150 45" << std::endl;
        return 1;
    }

    // Parse arguments
    int x_pos = std::atoi(argv[1]);
    int y_pos = std::atoi(argv[2]);
    float heading_deg = std::atof(argv[3]);
    
    // Convert heading from degrees to radians
    float heading_rad = deg_to_rad(heading_deg);
    
    // Create position object
    Pos position(x_pos, y_pos, heading_rad);
    
    // Read the first frame from video
    cv::VideoCapture cap("480p.mp4");
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return -1;
    }
    
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        std::cerr << "Error: Could not read the first frame." << std::endl;
        return -1;
    }
    
    // Create processor
    camera::CamProcessor processor;
    
    // Calculate and output raw loss
    float loss = processor.calculate_loss(frame, position);
    
    // Print just the raw loss value
    std::cout << loss << std::endl;
    
    return 0;
}