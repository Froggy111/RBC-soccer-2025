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
        
        // Find ball position and heading
        cv::Point ballPosition;
        double ballHeading;
        bool ballDetected = detector.detectBall(frame, ballPosition, ballHeading);
        
        // Create output visualization
        cv::Mat output = frame.clone();
        detector.drawDebugInfo(output, currentFramePoints, ballDetected ? &ballPosition : nullptr);
        
        // Debug visualization
        if (DEBUG) {
            cv::Mat debugView = detector.createDebugView(
                frame, irMask, currentFramePoints, ballDetected ? &ballPosition : nullptr);
            cv::imshow("IR Detection Debug", debugView);
        }
        
        cv::imshow("IR Detection", output);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Frame " << frameCount << ": Found " << currentFramePoints.size() 
                  << " points in " << duration.count() << "ms" << std::endl;
        
        // Print points in current frame
        if (!currentFramePoints.empty()) {
            std::cout << "Points in frame " << frameCount << ":" << std::endl;
            for (size_t i = 0; i < currentFramePoints.size(); i++) {
                const auto& point = currentFramePoints[i];
                std::cout << "  Point " << (i+1) << ": Position (" 
                          << point.position.x << "," << point.position.y 
                          << "), Angle: " << point.angle << "°"
                          << ", Distance: " << point.distance << std::endl;
            }
        }
        
        if (ballDetected) {
            std::cout << "Ball detected at (" << ballPosition.x << "," << ballPosition.y 
                      << ") with heading " << ballHeading << "°" << std::endl;
        }
        
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