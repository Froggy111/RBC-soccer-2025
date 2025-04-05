#include "ball.hpp"

BallDetector::BallDetector(const cv::Point &centerPoint, int minContourArea,
                           int minBrightness, bool debug)
    : m_centerPoint(centerPoint), m_minContourArea(minContourArea),
      m_minBrightness(minBrightness), m_debug(debug),
      m_purpleLower(cv::Scalar(130, 50, 100)),
      m_purpleUpper(cv::Scalar(175, 255, 255)) {}

bool BallDetector::detectBall(const cv::Mat &frame, cv::Point &ballPosition,
                              double &ballHeading) {
    cv::Mat irMask;
    std::vector<IRPoint> irPoints = detectIRPoints(frame, irMask);

    if (irPoints.empty()) {
        return false;
    }

    // Find strongest point (assumed to be the ball)
    ballPosition = findStrongestIRSource(irPoints);

    // Find the point data for the strongest point
    for (const auto &point : irPoints) {
        if (point.position == ballPosition) {
            ballHeading = point.angle;
            return true;
        }
    }

    return false;
}

std::vector<IRPoint> BallDetector::detectIRPoints(const cv::Mat &frame,
                                                  cv::Mat &irMask) {
    std::vector<IRPoint> irPoints;

    // Convert to HSV colorspace
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // Create mask for purple color
    cv::Mat purpleMask;
    cv::inRange(hsv, m_purpleLower, m_purpleUpper, purpleMask);

    // Check for brightness in original frame
    cv::Mat gray, brightMask;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, brightMask, m_minBrightness, 255, cv::THRESH_BINARY);

    // Combine masks - we want purple AND bright
    cv::bitwise_and(purpleMask, brightMask, irMask);

    // Apply minimal morphology to clean up the mask
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(irMask, irMask, cv::MORPH_OPEN, kernel);

    // Find contours in the mask
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(irMask.clone(), contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    // Filter contours by minimum area
    for (const auto &contour : contours) {
        double area = cv::contourArea(contour);
        if (area >= m_minContourArea) {
            // Calculate centroid of the contour
            cv::Moments M = cv::moments(contour);
            if (M.m00 > 0) {
                int cx = static_cast<int>(M.m10 / M.m00);
                int cy = static_cast<int>(M.m01 / M.m00);
                cv::Point position(cx, cy);

                IRPoint point;
                point.position  = position;
                point.intensity = area;
                point.angle     = calculateAngle(position);
                point.distance  = calculateDistance(position, m_centerPoint);
                irPoints.push_back(point);
            }
        }
    }

    return irPoints;
}

double BallDetector::calculateDistance(const cv::Point &p1,
                                       const cv::Point &p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

double BallDetector::calculateAngle(const cv::Point &point) {
    double deltaX = point.x - m_centerPoint.x;
    double deltaY = point.y - m_centerPoint.y;
    double angle  = std::atan2(deltaY, deltaX);

    // Convert to degrees and adjust to match the Python code
    return (angle + CV_PI / 2) * 180.0 / CV_PI;
}

cv::Point
BallDetector::findStrongestIRSource(const std::vector<IRPoint> &irPoints) {
    if (irPoints.empty()) {
        return cv::Point(-1, -1); // Invalid point
    }

    // Sort by intensity (find max)
    auto maxPoint = std::max_element(irPoints.begin(), irPoints.end(),
                                     [](const IRPoint &a, const IRPoint &b) {
                                         return a.intensity < b.intensity;
                                     });

    return maxPoint->position;
}

void BallDetector::drawDebugInfo(cv::Mat &outputFrame,
                                 const std::vector<IRPoint> &points,
                                 const cv::Point *strongestPoint) {
    // Mark center point
    cv::circle(outputFrame, m_centerPoint, 5, cv::Scalar(255, 255, 255), -1);

    // Draw all detected IR points
    for (const auto &point : points) {
        // Size of circle based on intensity
        int radius =
            std::max(5, std::min(20, static_cast<int>(point.intensity / 10)));

        // Draw circle
        cv::circle(outputFrame, point.position, radius, cv::Scalar(255, 0, 255),
                   -1);

        // Draw line from center to point
        cv::line(outputFrame, m_centerPoint, point.position,
                 cv::Scalar(255, 0, 255), 2);

        // Display information
        cv::putText(outputFrame,
                    "(" + std::to_string(point.position.x) + "," +
                        std::to_string(point.position.y) + ")",
                    cv::Point(point.position.x - 40, point.position.y - 15),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 1);

        cv::putText(
            outputFrame,
            "Angle: " +
                std::to_string(static_cast<int>(point.angle * 10) / 10.0) + "°",
            cv::Point(point.position.x - 40, point.position.y + 15),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 1);

        cv::putText(
            outputFrame,
            "Dist: " +
                std::to_string(static_cast<int>(point.distance * 10) / 10.0),
            cv::Point(point.position.x - 40, point.position.y + 30),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 1);
    }

    // Highlight the strongest IR source
    if (strongestPoint != nullptr && strongestPoint->x >= 0) {
        cv::circle(outputFrame, *strongestPoint, 15, cv::Scalar(0, 255, 255),
                   2);
        cv::putText(outputFrame, "Strongest IR",
                    cv::Point(strongestPoint->x - 40, strongestPoint->y - 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
    }
}

cv::Mat BallDetector::createDebugView(const cv::Mat &frame,
                                      const cv::Mat &irMask,
                                      const std::vector<IRPoint> &points,
                                      const cv::Point *strongestPoint) {
    // Create output frame
    cv::Mat output = frame.clone();
    drawDebugInfo(output, points, strongestPoint);

    // Create mask visualization
    cv::Mat irDisplay;
    cv::cvtColor(irMask, irDisplay, cv::COLOR_GRAY2BGR);
    cv::putText(irDisplay, "IR Mask", cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

    // Information display
    cv::Mat infoDisplay = cv::Mat::zeros(frame.size(), frame.type());
    cv::putText(infoDisplay,
                "Points in current frame: " + std::to_string(points.size()),
                cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(0, 255, 0), 2);

    if (strongestPoint != nullptr && strongestPoint->x >= 0) {
        // Find the point data for the strongest point
        for (const auto &point : points) {
            if (point.position == *strongestPoint) {
                cv::putText(infoDisplay,
                            "Strongest IR Angle: " +
                                std::to_string(
                                    static_cast<int>(point.angle * 10) / 10.0) +
                                "°",
                            cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                            cv::Scalar(0, 255, 0), 2);

                cv::putText(
                    infoDisplay,
                    "Strongest IR Distance: " +
                        std::to_string(static_cast<int>(point.distance * 10) /
                                       10.0),
                    cv::Point(10, 110), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                    cv::Scalar(0, 255, 0), 2);
                break;
            }
        }
    }

    // Combine displays
    int h = frame.rows;
    int w = frame.cols;

    cv::Mat irDisplayResized, infoDisplayResized;
    cv::resize(irDisplay, irDisplayResized, cv::Size(w / 2, h / 2));
    cv::resize(infoDisplay, infoDisplayResized, cv::Size(w / 2, h / 2));

    cv::Mat debugTop, outputResized, debugView;
    cv::hconcat(std::vector<cv::Mat>{irDisplayResized, infoDisplayResized},
                debugTop);
    cv::resize(output, outputResized, cv::Size(w, h / 2));
    cv::vconcat(std::vector<cv::Mat>{debugTop, outputResized}, debugView);

    return debugView;
}

IRPoint
BallDetector::detectIRPointByHeading(const std::vector<IRPoint> &irPoints,
                                     double heading, double angleTolerance) {
    // Default return value (invalid point)
    IRPoint result;
    result.position  = cv::Point(-1, -1);
    result.intensity = 0;
    result.angle     = 0;
    result.distance  = 0;

    // If no points, return invalid point
    if (irPoints.empty()) {
        return result;
    }

    double maxIntensity = 0;
    bool foundPoint     = false;

    // Filter points by heading and find max intensity
    for (const auto &point : irPoints) {
        // Calculate angle difference, handling wraparound
        double angleDiff = std::abs(point.angle - heading);
        if (angleDiff > 180) {
            angleDiff = 360 - angleDiff;
        }

        // Check if within tolerance
        if (angleDiff <= angleTolerance) {
            if (!foundPoint || point.intensity > maxIntensity) {
                maxIntensity = point.intensity;
                result       = point;
                foundPoint   = true;
            }
        }
    }

    return result;
}

IRPoint BallDetector::detectIRPoint(const std::vector<IRPoint> &irPoints,
                                    float headingRadians) {
    // Default return value (invalid point)
    IRPoint result;
    result.position  = cv::Point(-1, -1);
    result.intensity = 0;
    result.angle     = 0;
    result.distance  = 0;

    // If no points, return invalid point
    if (irPoints.empty()) {
        return result;
    }

    // Convert heading from radians to degrees since our angle property is in degrees
    float headingDegrees = headingRadians * 180.0 / CV_PI;

    // Fixed tolerance (0.26 radians converted to degrees)
    float toleranceDegrees = 0.26 * 180.0 / CV_PI;

    float maxIntensity = 0;
    bool foundPoint     = false;

    // Shortlist points within tolerance of the heading
    for (const auto &point : irPoints) {
        // Calculate angle difference, handling wraparound
        float angleDiff = std::abs(point.angle - headingDegrees);
        if (angleDiff > 180) {
            angleDiff = 360 - angleDiff;
        }

        // Check if within tolerance
        if (angleDiff <= toleranceDegrees) {
            // Select the point with highest intensity
            if (!foundPoint || point.intensity > maxIntensity) {
                maxIntensity = point.intensity;
                result       = point;
                foundPoint   = true;
            }
        }
    }

    return result;
}

double BallDetector::getDistanceToBall(const IRPoint &irPoint) {
    // Check if the point is valid (has a valid position)
    if (irPoint.position.x < 0 || irPoint.position.y < 0) {
        return -1.0; // Return negative value to indicate invalid point
    }

    // Calculate the distance between the center point and the ball position
    return calculateDistance(m_centerPoint, irPoint.position);
}

IRPoint BallDetector::getBallInfo(const cv::Mat &frame, double headingRadians,
                                  cv::Mat *outputFrame) {
    // Initialize default result (invalid point)
    IRPoint ballInfo;
    ballInfo.position  = cv::Point(-1, -1);
    ballInfo.intensity = 0;
    ballInfo.angle     = 0;
    ballInfo.distance  = 0;

    // Create a copy for output visualization if requested
    cv::Mat output;
    if (outputFrame) {
        output = frame.clone();
        cv::circle(output, m_centerPoint, 5, cv::Scalar(255, 255, 255), -1);
    }

    // Detect IR points in the frame
    cv::Mat irMask;
    std::vector<IRPoint> irPoints = detectIRPoints(frame, irMask);

    // If no points were detected, return the default (invalid) result
    if (irPoints.empty()) {
        if (outputFrame) {
            *outputFrame = output;
        }
        return ballInfo;
    }

    // Find the IR point based on heading
    ballInfo = detectIRPoint(irPoints, headingRadians);

    // Visualize if output requested
    if (outputFrame) {
        // If we found a valid ball point, highlight it
        if (ballInfo.position.x >= 0) {
            // Draw all detected points with less emphasis
            for (const auto &point : irPoints) {
                if (point.position != ballInfo.position) {
                    // Draw smaller circles for non-selected points
                    cv::circle(output, point.position, 3,
                               cv::Scalar(100, 0, 100), -1);
                }
            }

            // Draw the selected ball point with emphasis
            int radius = std::max(
                5, std::min(20, static_cast<int>(ballInfo.intensity / 10)));
            cv::circle(output, ballInfo.position, radius,
                       cv::Scalar(255, 0, 255), -1);
            cv::circle(output, ballInfo.position, radius + 5,
                       cv::Scalar(0, 255, 255), 2);

            // Draw line from center to ball
            cv::line(output, m_centerPoint, ballInfo.position,
                     cv::Scalar(255, 0, 255), 2);

            // Display information
            cv::putText(
                output,
                "Ball: (" + std::to_string(ballInfo.position.x) + "," +
                    std::to_string(ballInfo.position.y) + ")",
                cv::Point(ballInfo.position.x - 40, ballInfo.position.y - 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

            cv::putText(
                output,
                "Angle: " +
                    std::to_string(static_cast<int>(ballInfo.angle * 10) /
                                   10.0) +
                    "°",
                cv::Point(ballInfo.position.x - 40, ballInfo.position.y + 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

            cv::putText(
                output,
                "Dist: " + std::to_string(
                               static_cast<int>(ballInfo.distance * 10) / 10.0),
                cv::Point(ballInfo.position.x - 40, ballInfo.position.y + 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

            // Display the heading we were searching for
            cv::putText(
                output,
                "Target heading: " +
                    std::to_string(
                        static_cast<int>(headingRadians * 180.0 / CV_PI * 10) /
                        10.0) +
                    "°",
                cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(0, 255, 0), 2);
        } else {
            // No ball found at the specified heading
            cv::putText(
                output,
                "No ball found at heading: " +
                    std::to_string(
                        static_cast<int>(headingRadians * 180.0 / CV_PI * 10) /
                        10.0) +
                    "°",
                cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(0, 0, 255), 2);
        }

        // Set output image
        *outputFrame = output;
    }

    return ballInfo;
}