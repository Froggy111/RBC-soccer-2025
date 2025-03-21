#include "camera.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cstdio>
#include "processor.hpp"

Camera camera;
CamProcessor processor;

int main() {
    // Initialize with desired resolution
    if (!camera.initialize(cam_config::RES_480P)) {
        fprintf(stderr, "Failed to initialize camera\n");
        return 1;
    }
    
    // Start capturing with our frame processor
    if (!camera.startCapture(processor.processFrame)) {
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