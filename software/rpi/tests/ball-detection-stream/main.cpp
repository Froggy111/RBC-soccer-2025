#include "ball.hpp"
#include "camera.hpp"
#include "config.hpp"
#include "debug.hpp"
#include <atomic>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <signal.h>
#include <thread>

// Global flag for clean shutdown
std::atomic<bool> running(true);

// Signal handler for clean termination
void signalHandler(int signum) {
    std::cout << "Interrupt signal received. Exiting..." << std::endl;
    running = false;
}

// Process camera frames and detect the ball
void process_camera(cv::Mat frame) {
    // Initialize ball detector
    // Using the center of the image as reference point
    static BallDetector ballDetector(cv::Point(frame.cols / 2, frame.rows / 2));

    // Detect ball position and heading
    cv::Point ballPosition;
    double ballHeading;
    bool ballFound = ballDetector.detectBall(frame, ballPosition, ballHeading);

    // Get IR points and mask for visualization
    cv::Mat irMask;
    std::vector<IRPoint> irPoints = ballDetector.detectIRPoints(frame, irMask);

    // Create debug visualization
    cv::Mat debugView;
    if (ballFound) {
        debugView = ballDetector.createDebugView(frame, irMask, irPoints,
                                                 &ballPosition);
        debug::info("Ball detected at (%d, %d) with heading %.1f degrees",
                    ballPosition.x, ballPosition.y, ballHeading);
    } else {
        debugView =
            ballDetector.createDebugView(frame, irMask, irPoints, nullptr);
        debug::info("No ball detected");
    }

    // Display visualization
    cv::imshow("Ball Detection", debugView);
    cv::waitKey(1); // Process UI events
}

int main() {
    // Set up signal handling for clean shutdown with Ctrl+C
    signal(SIGINT, signalHandler);

    debug::info("Starting ball detection test...");

    // Initialize camera with 480P resolution for better performance
    camera::Camera cam;
    if (!cam.initialize(camera::RES_480P)) {
        std::cerr << "Failed to initialize camera" << std::endl;
        return 1;
    }

    debug::info("Camera initialized successfully");

    // Start capture with our processing function
    if (!cam.startCapture(process_camera)) {
        std::cerr << "Failed to start camera capture" << std::endl;
        return 1;
    }

    debug::info("Ball detection started. Press Ctrl+C to exit.");

    // Main loop - keep running until signal received
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up
    debug::info("Shutting down...");
    cam.stopCapture();
    cv::destroyAllWindows();

    return 0;
}