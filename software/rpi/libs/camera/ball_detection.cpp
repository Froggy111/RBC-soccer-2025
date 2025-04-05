#include "ball_detection.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Define constants
const bool DEBUG = true;
const bool SAVE_POINTS = false;
const int MIN_CONTOUR_AREA = 10;
const int MIN_BRIGHTNESS = 130;
const cv::Point CENTRE_POINT(640 / 2 - 5, 480 / 2 + 30);

// Purple HSV range
const cv::Scalar PURPLE_LOWER(130, 50, 100);  // Expanded range to capture more variations
const cv::Scalar PURPLE_UPPER(175, 255, 255);

// Calculate Euclidean distance between two points
double calculateDistance(const cv::Point& p1, const cv::Point& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

// Calculate angle from center to point (in radians)
double calculateAngle(const cv::Point& point, const cv::Point& center) {
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
                point.angle = calculateAngle(position, CENTRE_POINT) * 180.0 / M_PI;
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