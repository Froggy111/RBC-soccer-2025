#include "camera.hpp"
#include "processor.hpp"
#include <cstdio>
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

    while (true) {
        Pos current_pos = processor.current_pos;
        std::printf("Current Position: x: %d, y: %d, heading: %.2f\n",
                    current_pos.x, current_pos.y, current_pos.heading);

        // sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop capturing
    cam.stopCapture();
}