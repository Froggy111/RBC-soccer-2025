#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>

struct IRPoint {
    cv::Point position;
    double intensity;
    double angle;
    double distance;
};

class BallDetector {
public:
    // Constructor with default values
    BallDetector(
        const cv::Point& centerPoint = cv::Point(640/2 - 5, 480/2 + 30),
        int minContourArea = 10,
        int minBrightness = 130,
        bool debug = true
    );

    // Main detection function returning ball position and heading
    bool detectBall(const cv::Mat& frame, cv::Point& ballPosition, double& ballHeading);

    // Additional functions for debugging and visualization
    std::vector<IRPoint> detectIRPoints(const cv::Mat& frame, cv::Mat& irMask);
    void drawDebugInfo(cv::Mat& outputFrame, const std::vector<IRPoint>& points, const cv::Point* strongestPoint = nullptr);
    cv::Mat createDebugView(const cv::Mat& frame, const cv::Mat& irMask, const std::vector<IRPoint>& points, const cv::Point* strongestPoint = nullptr);

    // Setter methods for parameters
    void setCenterPoint(const cv::Point& center) { m_centerPoint = center; }
    void setMinContourArea(int area) { m_minContourArea = area; }
    void setMinBrightness(int brightness) { m_minBrightness = brightness; }
    void setDebugMode(bool debug) { m_debug = debug; }
    IRPoint detectIRPointByHeading(const std::vector<IRPoint>& irPoints, double heading, double angleTolerance);

private:
    // Configuration parameters
    cv::Point m_centerPoint;
    int m_minContourArea;
    int m_minBrightness;
    bool m_debug;

    // HSV color range for purple (IR points)
    cv::Scalar m_purpleLower;
    cv::Scalar m_purpleUpper;

    // Helper functions
    double calculateDistance(const cv::Point& p1, const cv::Point& p2);
    double calculateAngle(const cv::Point& point);
    cv::Point findStrongestIRSource(const std::vector<IRPoint>& irPoints);
};