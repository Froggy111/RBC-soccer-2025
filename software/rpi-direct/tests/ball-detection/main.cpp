#include <iostream>
#include <chrono>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "ball.hpp"

int main(int argc, char** argv) {
    const bool DEBUG = true;
    std::string video_path;
    
    // Check if video path was provided as command line argument
    if (argc > 1) {
        video_path = argv[1];
    } else {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "ball-detection-test") << " <video_path>" << std::endl;
        return -1;
    }
    
    // Check if file exists
    std::ifstream file(video_path);
    std::cout << "file exists? " << (file.good() ? "true" : "false") << std::endl;
    file.close();

    // Open video file
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video file: " << video_path << std::endl;
        return -1;
    }
    
    // Initialize ball detector
    BallDetector detector;
    detector.setDebugMode(DEBUG);
    
    // For statistics - can be removed if not needed
    int allFramesTotalPoints = 0;
    int frameCount = 0;
    
    // Fixed heading angle (40 degrees)
    const double fixedHeading = 40.0;
    const double angleTolerance = 15.0; // Accept points within ±15 degrees
    
    // Main loop
    while (true) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            break;
        }
        
        frameCount++;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Detect IR points
        cv::Mat irMask;
        std::vector<IRPoint> currentFramePoints = detector.detectIRPoints(frame, irMask);
        
        // Update statistics
        allFramesTotalPoints += currentFramePoints.size();
        
        // Create output visualization (initially without drawings)
        cv::Mat output = frame.clone();
        
        // Try to detect IR point by heading using fixed heading
        IRPoint headingBasedBall;
        bool headingBasedDetection = false;
        
        if (!currentFramePoints.empty()) {
            headingBasedBall = detector.detectIRPointByHeading(
                currentFramePoints, fixedHeading, angleTolerance);
                
            // If found a valid point by heading
            if (headingBasedBall.position.x >= 0) {
                headingBasedDetection = true;
                
                // Create a single-element vector with just the heading-based ball
                std::vector<IRPoint> singleBall = {headingBasedBall};
                
                // Use the existing drawDebugInfo method to draw just this one point
                detector.drawDebugInfo(output, singleBall, &headingBasedBall.position);
                
                std::cout << "Heading-based detection: Position (" 
                          << headingBasedBall.position.x << "," << headingBasedBall.position.y 
                          << "), Angle: " << headingBasedBall.angle << "°"
                          << ", Distance: " << headingBasedBall.distance
                          << ", Fixed heading: " << fixedHeading << "±" << angleTolerance << "°" << std::endl;
            }
        }
        
        // Debug visualization
        if (DEBUG) {
            cv::Mat debugView;
            if (headingBasedDetection) {
                // Create custom debug view only showing the heading-based ball
                std::vector<IRPoint> singleBall = {headingBasedBall};
                debugView = detector.createDebugView(frame, irMask, singleBall, &headingBasedBall.position);
            } else {
                // If no heading-based detection, show empty points
                std::vector<IRPoint> emptyPoints;
                debugView = detector.createDebugView(frame, irMask, emptyPoints, nullptr);
            }
            cv::imshow("IR Detection Debug", debugView);
        }
        
        cv::imshow("IR Detection", output);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Frame " << frameCount << ": Found " << currentFramePoints.size() 
                  << " points in " << duration.count() << "ms" << std::endl;
        
        if (cv::waitKey(25) == 'q') {
            break;
        }
    }
    
    // Print summary
    std::cout << "Total frames processed: " << frameCount << std::endl;
    std::cout << "Total points detected across all frames: " << allFramesTotalPoints << std::endl;
    
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
}