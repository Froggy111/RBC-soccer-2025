#ifndef GOALPOST_DETECTION_HPP
#define GOALPOST_DETECTION_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <chrono>


class GoalpostDetector {
private:
    // Configuration
    bool DEBUG;
    std::string videoPath;
    cv::VideoCapture cap;
    
    // HSV color ranges
    cv::Scalar BLUE_LOWER;
    cv::Scalar BLUE_UPPER;
    cv::Scalar YELLOW_LOWER;
    cv::Scalar YELLOW_UPPER;
    cv::Scalar FIELD_LOWER;
    cv::Scalar FIELD_UPPER;
    
    // Constants
    int MIN_CONTOUR_AREA;
    int MIN_GOAL_HEIGHT;
    int EDGE_SEARCH_HEIGHT;
    double EPSILON_FACTOR;
    int PARALLELOGRAM_TOLERANCE;
    cv::Point CENTRE_POINT;

    // Helper functions
    int findTrueBottomEdge(const cv::Mat& frame, const cv::Rect& bbox, 
                          const cv::Scalar& goalLower, const cv::Scalar& goalUpper);
                          
    int findTrueBottomEdgeParallelogram(const cv::Mat& frame, const cv::Mat& parallelogram, 
                                      const cv::Scalar& goalLower, const cv::Scalar& goalUpper);
                                      
    int findTrueBottomEdgeQuadrilateral(const cv::Mat& frame, const cv::Mat& quadrilateral, 
                                       const cv::Scalar& goalLower, const cv::Scalar& goalUpper);
                                       
    cv::Mat orderPoints(const cv::Mat& pts);
    
    cv::Mat detectQuadrilateral(const std::vector<cv::Point>& contour, double epsilonFactor = 0.03, 
                              int minHeight = 10, int maxPoints = 6);
                              
    cv::Mat orderQuadrilateralPoints(const cv::Mat& pts);
    
    cv::Point findNearestFieldPoint(const cv::Mat& quadrilateral, int bottomEdgeY, int frameHeight);
    
    std::vector<cv::Point> getReliableGoalpostContour(const std::vector<std::vector<cv::Point>>& contours, 
                                                   const cv::Mat& colorMask);
                                                   
    std::vector<std::vector<cv::Point>> filterOutRods(const std::vector<std::vector<cv::Point>>& contours);
    
    std::vector<cv::Point> combineGoalpostParts(const std::vector<std::vector<cv::Point>>& contours, int minArea);
    
    cv::Point2f findLineMidpoint(const std::vector<cv::Point>& linePoints);
    
    cv::Point2f findQuadMidpoint(const cv::Mat& quadPoints);
    
    double findGoalAngle(const cv::Point2f& goalMidpoint);
    
    std::vector<cv::Point> findClosestPointsToCenter(const cv::Mat& quadPoints, 
                                                 const cv::Point& centerPoint = cv::Point(0, 0));
                                                 
    double findDistanceToGoal(const cv::Point2f& goalMidpoint);

public:
    // Constructor with default parameters
    GoalpostDetector(const std::string& videoPath = "vid4.mp4", bool debug = true);
    
    // Destructor
    ~GoalpostDetector();
    
    // Initialize detector
    bool initialize();
    
    // Process a single frame
    cv::Mat processFrame(cv::Mat& frame, bool& blueDetected, bool& yellowDetected,
                       double& blueGoalAngle, double& yellowGoalAngle,
                       double& blueGoalDistance, double& yellowGoalDistance);
    
    // Run the detection on video
    void run();
};

#endif // GOALPOST_DETECTION_HPP
