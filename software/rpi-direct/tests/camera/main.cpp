#include "camera.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cstdio>

// Simple frame processor callback function
void processFrame(const cv::Mat& frame) {
    static int frameCount = 0;
    frameCount++;
    
    // Print frame info every 30 frames
    if (frameCount % 30 == 0) {
        printf("Processing frame #%d (%dx%d)\n", 
               frameCount, frame.cols, frame.rows);
    }
    
    // Optional: display the frame if in a GUI environment
    // cv::imshow("Camera Feed", frame);
    // cv::waitKey(1);
    
    // Optional: save a frame periodically
    if (frameCount % 100 == 0) {
        std::string filename = "frame_" + std::to_string(frameCount) + ".jpg";
        cv::imwrite(filename, frame);
        printf("Saved %s\n", filename.c_str());
    }
}

int main() {
    // Create camera instance
    Camera camera;
    
    // Initialize with desired resolution
    if (!camera.initialize(cam_config::RES_480P)) {
        fprintf(stderr, "Failed to initialize camera\n");
        return 1;
    }
    
    // Start capturing with our frame processor
    if (!camera.startCapture(processFrame)) {
        fprintf(stderr, "Failed to start camera capture\n");
        return 1;
    }
    
    printf("Camera is running. Press Enter to stop...\n");
    
    // Run for a specified duration (or wait for user input)
    std::cin.get();
    
    // Alternative: Run for a fixed duration
    // std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Stop capturing
    camera.stopCapture();
    
    printf("Camera test completed\n");
    return 0;
}