#include "camera.hpp"
#include "processor.hpp"
#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>

camera::Camera cam;
camera::CamProcessor processor;

int main() {
    // Initialize with desired resolution
    if (!cam.initialize(camera::RES_480P)) {
        fprintf(stderr, "Failed to initialize camera\n");
        return 1;
    }

    // Start capturing with our frame processor
    if (!cam.startCapture(processor.process_frame)) {
        fprintf(stderr, "Failed to start camera capture\n");
        return 1;
    }


    // Stop capturing
    cam.stopCapture();
}