#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>

namespace goalpost {

// Constants
const bool DEBUG = true;
const int MIN_CONTOUR_AREA = 300;
const int MIN_GOAL_HEIGHT = 10;
const int EDGE_SEARCH_HEIGHT = 69;
const double EPSILON_FACTOR = 0.05;
const double PARALLELOGRAM_TOLERANCE = 1.0;

// Color thresholds in HSV
const cv::Scalar BLUE_LOWER(70, 146, 50);
const cv::Scalar BLUE_UPPER(150, 255, 255);
const cv::Scalar YELLOW_LOWER(20, 100, 100);
const cv::Scalar YELLOW_UPPER(30, 255, 255);
const cv::Scalar FIELD_LOWER(35, 50, 50);
const cv::Scalar FIELD_UPPER(85, 255, 255);

// Function declarations
int findTrueBottomEdge(const cv::Mat& frame, const cv::Rect& bbox, 
                      const cv::Scalar& goalLower, const cv::Scalar& goalUpper);

int findTrueBottomEdgeQuadrilateral(const cv::Mat& frame, const std::vector<cv::Point2f>& quadrilateral,
                                  const cv::Scalar& goalLower, const cv::Scalar& goalUpper);

std::vector<cv::Point2f> orderPoints(const std::vector<cv::Point2f>& pts);

std::vector<cv::Point2f> detectQuadrilateral(const std::vector<cv::Point>& contour, 
                                          double epsilonFactor = EPSILON_FACTOR,
                                          int minHeight = MIN_GOAL_HEIGHT, 
                                          int maxPoints = 6);

std::vector<cv::Point2f> orderQuadrilateralPoints(const std::vector<cv::Point2f>& pts);

cv::Point2f findNearestFieldPoint(const std::vector<cv::Point2f>& quadrilateral, 
                                int bottomEdgeY, int frameHeight);

std::vector<cv::Point> getReliableGoalpostContour(const std::vector<std::vector<cv::Point>>& contours, 
                                               const cv::Mat& colorMask);

std::vector<std::vector<cv::Point>> filterOutRods(const std::vector<std::vector<cv::Point>>& contours);

std::vector<cv::Point> combineGoalpostParts(const std::vector<std::vector<cv::Point>>& contours, int minArea);

cv::Point2f findQuadMidpoint(const std::vector<cv::Point2f>& quadPoints);

}