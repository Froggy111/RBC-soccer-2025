#include <opencv2/opencv.hpp>
#include "processor.hpp"
#include "position.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>  // For timing measurements

int main() {
    // Path to input video and output file
    const std::string input_path = "./recordings/video_20250321_201254.h264";
    const std::string output_path = "./points_data.csv";
    
    // Open the input video file
    cv::VideoCapture cap(input_path);
    
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open input video file: " << input_path << std::endl;
        return -1;
    }
    
    // Get video properties for logging
    int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    
    std::cout << "Video properties:" << std::endl;
    std::cout << "  Width: " << frame_width << std::endl;
    std::cout << "  Height: " << frame_height << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
    
    // Open output file
    std::ofstream output_file(output_path);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not create output file: " << output_path << std::endl;
        return -1;
    }
    
    // Write CSV header - include timing information
    output_file << "frame,tl_x,tl_y,tl_loss,tr_x,tr_y,tr_loss,bl_x,bl_y,bl_loss,br_x,br_y,br_loss,processing_time_ms" << std::endl;
    
    // Set precision for floating point values
    output_file << std::fixed << std::setprecision(4);
    
    // Create a camera processor instance
    camera::CamProcessor processor;
    
    // Process each frame
    cv::Mat frame;
    int frame_count = 0;
    double total_time = 0.0;
    
    std::cout << "Processing frames..." << std::endl;
    
    // Process the video
    while (cap.read(frame)) {
        // Time the get_points function
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Extract the 4 points from the frame
        auto points = processor.get_points(frame);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        total_time += duration;
        
        // Unpack the tuple
        auto& top_left = std::get<0>(points);
        auto& top_right = std::get<1>(points);
        auto& bottom_left = std::get<2>(points);
        auto& bottom_right = std::get<3>(points);
        
        // Write data to file in CSV format (including timing)
        output_file << frame_count << ","
                   << top_left.first.x << "," << top_left.first.y << "," << top_left.second << ","
                   << top_right.first.x << "," << top_right.first.y << "," << top_right.second << ","
                   << bottom_left.first.x << "," << bottom_left.first.y << "," << bottom_left.second << ","
                   << bottom_right.first.x << "," << bottom_right.first.y << "," << bottom_right.second << ","
                   << duration
                   << std::endl;
        
        // Print timing information for this frame
        std::cout << "Frame " << frame_count << ": get_points() took " << duration 
                  << " ms (avg: " << (total_time / (frame_count + 1)) << " ms)" << std::endl;
        
        frame_count++;
    }
    
    // Close the output file
    output_file.close();
    
    // Print timing summary
    double avg_time = total_time / frame_count;
    std::cout << "\nPerformance summary:" << std::endl;
    std::cout << "  Total frames processed: " << frame_count << std::endl;
    std::cout << "  Total processing time: " << total_time << " ms" << std::endl;
    std::cout << "  Average time per frame: " << avg_time << " ms" << std::endl;
    std::cout << "  Theoretical max FPS: " << (1000.0 / avg_time) << std::endl;
    std::cout << "\nProcessing complete. Data saved to '" << output_path << "'" << std::endl;
    
    return 0;
}