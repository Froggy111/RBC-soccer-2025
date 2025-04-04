#ifndef BALL_DETECTION_HPP
#define BALL_DETECTION_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// Define struct to represent an IR point
struct IRPoint {
    cv::Point position;
    double intensity;
    double angle;
    double distance;
};

// Function declarations
double calculateDistance(const cv::Point& p1, const cv::Point& p2);
double calculateAngle(const cv::Point& point, const cv::Point& center);
std::string formatFloat(double value, int precision);
std::pair<std::vector<IRPoint>, cv::Mat> detectIRPoints(const cv::Mat& frame);
cv::Point findStrongestIRSource(const std::vector<IRPoint>& irPoints);

// Constants
extern const bool DEBUG;
extern const bool SAVE_POINTS;
extern const int MIN_CONTOUR_AREA;
extern const int MIN_BRIGHTNESS;
extern const cv::Point CENTRE_POINT;
extern const cv::Scalar PURPLE_LOWER;
extern const cv::Scalar PURPLE_UPPER;

#endif // BALL_DETECTION_HPP