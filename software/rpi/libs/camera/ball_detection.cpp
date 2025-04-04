#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Define struct to represent an IR point
struct IRPoint {
    cv::Point position;
    double intensity;
    double angle;
    double distance;
};

// Constants
const bool DEBUG = true;
const bool SAVE_POINTS = false;
const int MIN_CONTOUR_AREA = 10;
const int MIN_BRIGHTNESS = 130;
const cv::Point CENTRE_POINT(640 / 2 - 5, 480 / 2 + 30);

// Purple HSV range
const cv::Scalar PURPLE_LOWER(130, 50, 100);  // Expanded range to capture more variations
const cv::Scalar PURPLE_UPPER(175, 255, 255);

// Utility functions
double calculateDistance(const cv::Point& p1, const cv::Point& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

double calculateAngle(const cv::Point& point, const cv::Point& center = CENTRE_POINT) {
    double deltaX = point.x - center.x;
    double deltaY = point.y - center.y;
    double angle = std::atan2(deltaY, deltaX);
    return angle + M_PI / 2;
}

// Format floating point number to string with precision
std::string formatFloat(double value, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

// Detect bright purple (IR) points in the frame
std::pair<std::vector<IRPoint>, cv::Mat> detectIRPoints(const cv::Mat& frame) {
    // Convert to HSV colorspace
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    
    // Create mask for purple color
    cv::Mat purpleMask;
    cv::inRange(hsv, PURPLE_LOWER, PURPLE_UPPER, purpleMask);
    
    // Additionally check for brightness in original frame
    cv::Mat gray, brightMask;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, brightMask, MIN_BRIGHTNESS, 255, cv::THRESH_BINARY);
    
    // Combine masks - we want purple AND bright
    cv::Mat irMask;
    cv::bitwise_and(purpleMask, brightMask, irMask);
    
    // Apply minimal morphology to clean up the mask
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(irMask, irMask, cv::MORPH_OPEN, kernel);
    
    // Find contours in the mask
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(irMask.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Filter contours by minimum area
    std::vector<IRPoint> irPoints;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area >= MIN_CONTOUR_AREA) {
            // Calculate centroid of the contour
            cv::Moments M = cv::moments(contour);
            if (M.m00 > 0) {
                int cx = static_cast<int>(M.m10 / M.m00);
                int cy = static_cast<int>(M.m01 / M.m00);
                cv::Point position(cx, cy);
                
                // Create and store point data
                IRPoint point;
                point.position = position;
                point.intensity = area;
                point.angle = calculateAngle(position) * 180.0 / M_PI;
                point.distance = calculateDistance(position, CENTRE_POINT);
                
                irPoints.push_back(point);
            }
        }
    }
    
    return {irPoints, irMask};
}

// Find the strongest IR source based on intensity
cv::Point findStrongestIRSource(const std::vector<IRPoint>& irPoints) {
    if (irPoints.empty()) {
        return cv::Point(-1, -1); // Invalid point
    }
    
    // Find maximum intensity point
    auto maxElement = std::max_element(irPoints.begin(), irPoints.end(),
        [](const IRPoint& a, const IRPoint& b) {
            return a.intensity < b.intensity;
        });
    
    return maxElement->position;
}

int main() {
    // Open video file
    std::string videoPath = "vid2.mp4";
    
    // Check if file exists (C++ way)
    std::ifstream f(videoPath.c_str());
    std::cout << "file exists? " << (f.good() ? "true" : "false") << std::endl;
    f.close();
    
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return -1;
    }
    
    // Statistics
    int allFramesTotalPoints = 0;
    int frameCount = 0;
    
    // Main loop
    while (cap.isOpened()) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            break;
        }
        
        frameCount++;
        auto startTime = std::chrono::high_resolution_clock::now();
        cv::Mat output = frame.clone();
        
        // Mark center point
        cv::circle(output, CENTRE_POINT, 5, cv::Scalar(255, 255, 255), -1);
        
        // Detect IR points
        auto [currentFramePoints, irMask] = detectIRPoints(frame);
        
        // Update statistics
        allFramesTotalPoints += currentFramePoints.size();
        
        // Draw all detected IR points
        for (const auto& point : currentFramePoints) {
            // Size of circle based on intensity
            int radius = std::max(5, std::min(20, static_cast<int>(point.intensity / 10)));
            
            // Draw circle
            cv::circle(output, point.position, radius, cv::Scalar(255, 0, 255), -1);
            
            // Draw line from center to point
            cv::line(output, CENTRE_POINT, point.position, cv::Scalar(255, 0, 255), 2);
            
            // Display information
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
        cv::Point strongestPoint = findStrongestIRSource(currentFramePoints);
        if (strongestPoint.x >= 0) {
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
            int detectedCount = currentFramePoints.size();
            
            cv::putText(
                infoDisplay,
                "Points in current frame: " + std::to_string(detectedCount),
                cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2
            );
            
            cv::putText(
                infoDisplay,
                "Current frame: " + std::to_string(frameCount),
                cv::Point(10, 150),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2
            );
            
            // Find the point data for the strongest point
            if (strongestPoint.x >= 0) {
                auto strongestData = std::find_if(
                    currentFramePoints.begin(), currentFramePoints.end(),
                    [&strongestPoint](const IRPoint& p) {
                        return p.position.x == strongestPoint.x && p.position.y == strongestPoint.y;
                    }
                );
                
                if (strongestData != currentFramePoints.end()) {
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
                          << "), Angle: " << formatFloat(point.angle, 1) 
                          << "°, Distance: " << formatFloat(point.distance, 1) << std::endl;
            }
        }
        
        int key = cv::waitKey(25);
        if (key == 'q') {
            break;
        }
    }
    
    // Print summary
    std::cout << "Total frames processed: " << frameCount << std::endl;
    std::cout << "Total points detected across all frames: " << allFramesTotalPoints << std::endl;
    
    cap.release();
    cv::destroyAllWindows();
    
    // In C++, we would return the points through a function or use a global variable
    // if this were part of a larger program, but for a standalone script we print them
    std::cout << "All points in the last frame are stored in memory" << std::endl;
    
    return 0;
}