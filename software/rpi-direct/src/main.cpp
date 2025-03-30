#include "camera.hpp"
#include "motion.hpp"
#include "processor.hpp"
#include <cstdio>
#include <opencv2/opencv.hpp>

camera::Camera cam;
camera::CamProcessor processor;
MotionController motion_controller;

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

    motion_controller.startControlThread();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop capturing
    cam.stopCapture();
}