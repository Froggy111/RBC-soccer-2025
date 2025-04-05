#ifndef GOALPOST_DETECTOR_HPP
#define GOALPOST_DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>

/**
 * @brief Structure to hold goalpost detection results
 */
struct GoalpostInfo {
    bool detected;                  // Whether the goalpost was detected
    cv::Point2f midpoint;           // Midpoint of the goalpost
    float angle;                    // Angle in radians from center
    float distance;                 // Distance in pixels from center
    std::vector<cv::Point> quad;    // Quadrilateral points of the goalpost
};

/**
 * @brief Class for detecting blue and yellow goalposts in images
 */
class GoalpostDetector {
public:
    /**
     * @brief Constructor with optional debug mode
     * @param debug Enable debug visualization
     */
    GoalpostDetector(bool debug = false);

    /**
     * @brief Set the center point for angle calculations
     * @param center The center point
     */
    void setCenterPoint(const cv::Point& center);

    /**
     * @brief Detect goalposts in a frame
     * @param frame Input image frame
     * @param outputFrame Optional output frame with visualization
     * @return Pair of GoalpostInfo for blue and yellow goalposts
     */
    std::pair<GoalpostInfo, GoalpostInfo> detectGoalposts(
        const cv::Mat& frame, 
        cv::Mat* outputFrame = nullptr
    );

private:
    // HSV color thresholds
    cv::Scalar BLUE_LOWER;
    cv::Scalar BLUE_UPPER;
    cv::Scalar YELLOW_LOWER;
    cv::Scalar YELLOW_UPPER;
    cv::Scalar FIELD_LOWER;
    cv::Scalar FIELD_UPPER;

    // Parameters
    bool m_debug;
    int MIN_CONTOUR_AREA;
    int MIN_GOAL_HEIGHT;
    int EDGE_SEARCH_HEIGHT;
    float EPSILON_FACTOR;
    cv::Point m_centerPoint;

    // Helper methods
    std::vector<cv::Point> detectQuadrilateral(const std::vector<cv::Point>& contour);
    int findTrueBottomEdge(const cv::Mat& frame, const std::vector<cv::Point>& quad, 
                          const cv::Scalar& goalLower, const cv::Scalar& goalUpper);
    std::vector<cv::Point> filterOutRods(const std::vector<std::vector<cv::Point>>& contours);
    std::vector<cv::Point> combineGoalpostParts(const std::vector<std::vector<cv::Point>>& contours);
    std::vector<cv::Point> getReliableGoalpostContour(const std::vector<std::vector<cv::Point>>& contours);
    std::pair<cv::Point, cv::Point> findClosestPointsToCenter(const std::vector<cv::Point>& quad);
    cv::Point findQuadMidpoint(const std::vector<cv::Point>& quad);
    float findGoalAngle(const cv::Point& goalMidpoint);
    float findDistanceToGoal(const cv::Point& goalMidpoint);
    std::vector<cv::Point> orderQuadrilateralPoints(std::vector<cv::Point> pts);
    cv::Point findNearestFieldPoint(const std::vector<cv::Point>& quad, int bottomEdgeY, int frameHeight);
};

#endif // GOALPOST_DETECTOR_HPP