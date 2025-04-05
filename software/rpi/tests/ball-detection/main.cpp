#include "ball_detection.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <ctime>
#include <iomanip>

// Function to get current timestamp as string
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

int main() {
    // Open video file
    std::string videoPath = "vid2.mp4";
    std::cout << "File exists? " << std::filesystem::exists(videoPath) << std::endl;
    
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file: " << videoPath << std::endl;
        return -1;
    }
    
    // Statistics
    int totalFrames = 0;
    int totalPointsDetected = 0;
    std::vector<IRPoint> lastFramePoints;
    
    // Main processing loop
    while (cap.isOpened()) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            break; // End of video
        }
        
        totalFrames++;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Create output frame (copy of input)
        cv::Mat output = frame.clone();
        
        // Mark center point
        cv::circle(output, CENTRE_POINT, 5, cv::Scalar(255, 255, 255), -1);
        
        // Detect IR points in the current frame
        auto [irPoints, irMask] = detectIRPoints(frame);
        lastFramePoints = irPoints; // Save points from this frame
        
        // Update statistics
        totalPointsDetected += irPoints.size();
        
        // Draw all detected IR points
        for (const auto& point : irPoints) {
            // Calculate circle size based on intensity
            int radius = std::max(5, std::min(20, static_cast<int>(point.intensity / 10)));
            
            // Draw circle at point location
            cv::circle(output, point.position, radius, cv::Scalar(255, 0, 255), -1);
            
            // Draw line from center to point
            cv::line(output, CENTRE_POINT, point.position, cv::Scalar(255, 0, 255), 2);
            
            // Display point information
            std::string posText = "(" + std::to_string(point.position.x) + "," + 
                                  std::to_string(point.position.y) + ")";
            cv::putText(
                output, posText,
                cv::Point(point.position.x - 40, point.position.y - 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 1
            );
            
            std::string angleText = "Angle: " + formatFloat(point.angle, 1) + "°";
            cv::putText(
                output, angleText,
                cv::Point(point.position.x - 40, point.position.y + 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 1
            );
            
            std::string distText = "Dist: " + formatFloat(point.distance, 1);
            cv::putText(
                output, distText,
                cv::Point(point.position.x - 40, point.position.y + 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 1
            );
        }
        
        // Find and highlight the strongest IR source
        cv::Point strongestPoint = findStrongestIRSource(irPoints);
        if (strongestPoint.x >= 0) { // Valid point
            cv::circle(output, strongestPoint, 15, cv::Scalar(0, 255, 255), 2);
            cv::putText(
                output, "Strongest IR",
                cv::Point(strongestPoint.x - 40, strongestPoint.y - 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1
            );
        }
        
        // Debug visualization
        if (DEBUG) {
            // Create mask visualization
            cv::Mat irDisplay;
            cv::cvtColor(irMask, irDisplay, cv::COLOR_GRAY2BGR);
            cv::putText(
                irDisplay, "IR Mask",
                cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2
            );
            
            // Information display
            cv::Mat infoDisplay = cv::Mat::zeros(frame.size(), frame.type());
            int detectedCount = irPoints.size();
            
            cv::putText(
                infoDisplay,
                "Points in current frame: " + std::to_string(detectedCount),
                cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2
            );
            
            cv::putText(
                infoDisplay,
                "Current frame: " + std::to_string(totalFrames),
                cv::Point(10, 150),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2
            );
            
            // Display strongest point information if available
            if (strongestPoint.x >= 0) {
                auto strongestData = std::find_if(
                    irPoints.begin(), irPoints.end(),
                    [&strongestPoint](const IRPoint& p) {
                        return p.position.x == strongestPoint.x && p.position.y == strongestPoint.y;
                    }
                );
                
                if (strongestData != irPoints.end()) {
                    cv::putText(
                        infoDisplay,
                        "Strongest IR Angle: " + formatFloat(strongestData->angle, 1) + "°",
                        cv::Point(10, 70),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2
                    );
                    
                    cv::putText(
                        infoDisplay,
                        "Strongest IR Distance: " + formatFloat(strongestData->distance, 1),
                        cv::Point(10, 110),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2
                    );
                }
            }
            
            // Combine displays
            int h = frame.rows, w = frame.cols;
            cv::Mat irDisplayResized, infoDisplayResized;
            cv::resize(irDisplay, irDisplayResized, cv::Size(w / 2, h / 2));
            cv::resize(infoDisplay, infoDisplayResized, cv::Size(w / 2, h / 2));
            
            cv::Mat debugTop;
            cv::hconcat(irDisplayResized, infoDisplayResized, debugTop);
            
            cv::Mat outputResized;
            cv::resize(output, outputResized, cv::Size(w, h / 2));
            
            cv::Mat debugView;
            cv::vconcat(debugTop, outputResized, debugView);
            
            cv::imshow("IR Detection Debug", debugView);
        }
        
        // Show main output window
        cv::imshow("IR Detection", output);
        
        // Calculate and display processing time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Frame " << totalFrames << ": Found " << irPoints.size() 
                  << " points in " << duration.count() << "ms" << std::endl;
        
        // Print detailed information about detected points
        if (!irPoints.empty()) {
            std::cout << "Points in frame " << totalFrames << ":" << std::endl;
            for (size_t i = 0; i < irPoints.size(); i++) {
                const auto& point = irPoints[i];
                std::cout << "  Point " << (i+1) << ": Position (" 
                          << point.position.x << "," << point.position.y 
                          << "), Angle: " << formatFloat(point.angle, 1) 
                          << "°, Distance: " << formatFloat(point.distance, 1) << std::endl;
            }
        }
        
        // Wait for key press (25ms) - allows ~40fps video playback
        int key = cv::waitKey(25);
        if (key == 'q' || key == 27) { // 'q' or ESC to quit
            break;
        }
    }
    
    // Print summary statistics
    std::cout << "Total frames processed: " << totalFrames << std::endl;
    std::cout << "Total points detected across all frames: " << totalPointsDetected << std::endl;
    
    // Release resources
    cap.release();
    cv::destroyAllWindows();
    
    // Print information about the final frame points
    std::cout << "All " << lastFramePoints.size() 
              << " points in the last frame are stored in 'lastFramePoints' variable" << std::endl;
    
    return 0;
}