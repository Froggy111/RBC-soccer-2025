#include "camera.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cstdio>

void processFrame(const cv::Mat& frame) {
    static int frameCount = 0;
    static auto lastTime = std::chrono::high_resolution_clock::now();
    static float fps = 0.0f;
    
    frameCount++;
    
    // Calculate FPS every 30 frames
    if (frameCount % 30 == 0) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(currentTime - lastTime).count();
        fps = 30.0f / elapsed;
        lastTime = currentTime;
        
        printf("FPS: %.2f | Frame size: %dx%d\n", fps, frame.cols, frame.rows);
    }

    // Save a frame periodically
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
    
    // Stop capturing
    camera.stopCapture();
    
    printf("Camera test completed\n");
    return 0;
}