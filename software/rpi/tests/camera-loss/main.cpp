#include "processor.hpp"
#include "position.hpp"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <string>
#include <iostream>
#include <chrono>

// Function to read the first frame from a video file
cv::Mat readFirstFrame(const std::string& video_path) {
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file " << video_path << std::endl;
        return cv::Mat();
    }
    
    cv::Mat frame;
    cap >> frame;
    return frame;
}

int main(int argc, char** argv) {
    // Set up parameters
    const int field_width = 291;
    const int field_height = 350;
    const std::string video_path = "480p.mp4";
    
    // Initialize processor
    camera::CamProcessor processor;
    
    // Read the first frame
    std::cout << "Reading first frame from " << video_path << "..." << std::endl;
    cv::Mat test_frame = readFirstFrame(video_path);
    if (test_frame.empty()) {
        std::cerr << "Failed to read the first frame. Exiting." << std::endl;
        return 1;
    }
    
    std::cout << "Frame dimensions: " << test_frame.cols << "x" << test_frame.rows << std::endl;
    
    // Create empty images for the heatmaps
    cv::Mat heatmap_90(field_height, field_width, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat heatmap_270(field_height, field_width, CV_8UC3, cv::Scalar(0, 0, 0));
    
    // Timer for performance measurement
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Generate heatmap for heading 90 degrees (π/2)
    std::cout << "Generating heatmap for heading 90 degrees..." << std::endl;
    for (int y = 0; y < field_height; y++) {
        for (int x = 0; x < field_width; x++) {
            Pos position(x, y, M_PI / 2); // 90 degrees
            float loss = processor.calculate_loss(test_frame, position);
            
            // Convert loss to a color (blue to red spectrum)
            // Low loss (good match) = blue, high loss (bad match) = red
            int blue = static_cast<int>((1.0f - loss) * 255);
            int red = static_cast<int>(loss * 255);
            
            heatmap_90.at<cv::Vec3b>(y, x) = cv::Vec3b(blue, 0, red);
        }
        
        // Print progress
        if (y % 35 == 0) {
            std::cout << "Progress: " << (y * 100 / field_height) << "%" << std::endl;
        }
    }
    
    // Generate heatmap for heading 270 degrees (3π/2)
    std::cout << "Generating heatmap for heading 270 degrees..." << std::endl;
    for (int y = 0; y < field_height; y++) {
        for (int x = 0; x < field_width; x++) {
            Pos position(x, y, 3 * M_PI / 2); // 270 degrees
            float loss = processor.calculate_loss(test_frame, position);
            
            // Convert loss to a color (blue to red spectrum)
            int blue = static_cast<int>((1.0f - loss) * 255);
            int red = static_cast<int>(loss * 255);
            
            heatmap_270.at<cv::Vec3b>(y, x) = cv::Vec3b(blue, 0, red);
        }
        
        // Print progress
        if (y % 35 == 0) {
            std::cout << "Progress: " << (y * 100 / field_height) << "%" << std::endl;
        }
    }
    
    // Calculate elapsed time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    std::cout << "Heatmap generation completed in " << duration << " seconds." << std::endl;
    
    // Save the heatmap images
    std::cout << "Saving heatmap images..." << std::endl;
    
    // Save raw heatmaps
    bool success_90 = cv::imwrite("loss_heatmap_heading_90.png", heatmap_90);
    bool success_270 = cv::imwrite("loss_heatmap_heading_270.png", heatmap_270);
    
    if (!success_90 || !success_270) {
        std::cerr << "Failed to save one or more heatmap images." << std::endl;
    }
    
    // Apply color map for better visualization and save
    cv::Mat colored_90, colored_270;
    cv::applyColorMap(heatmap_90, colored_90, cv::COLORMAP_JET);
    cv::applyColorMap(heatmap_270, colored_270, cv::COLORMAP_JET);
    
    cv::imwrite("loss_heatmap_heading_90_colored.png", colored_90);
    cv::imwrite("loss_heatmap_heading_270_colored.png", colored_270);
    
    std::cout << "All operations completed successfully." << std::endl;
    
    return 0;
}