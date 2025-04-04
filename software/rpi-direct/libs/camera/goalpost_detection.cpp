#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <string>

// Constants
const bool DEBUG = true;

// Color thresholds (H:0-180, S:0-255, V:0-255)
const cv::Scalar BLUE_LOWER(100, 50, 50);
const cv::Scalar BLUE_UPPER(150, 255, 255);
const cv::Scalar YELLOW_LOWER(20, 137, 110);
const cv::Scalar YELLOW_UPPER(30, 255, 255);
const cv::Scalar FIELD_LOWER(35, 50, 50);
const cv::Scalar FIELD_UPPER(85, 255, 255);

const int MIN_CONTOUR_AREA = 400;
const int MIN_GOAL_HEIGHT = 10;
const int EDGE_SEARCH_HEIGHT = 69;
const float EPSILON_FACTOR = 0.05f;
const int PARALLELOGRAM_TOLERANCE = 1;
const cv::Point CENTRE_POINT(640 / 2 - 5, 480 / 2 + 30);

// Function prototypes
int findTrueBottomEdge(const cv::Mat& frame, const cv::Rect& bbox, 
                      const cv::Scalar& goalLower, const cv::Scalar& goalUpper);
int findTrueBottomEdgeQuadrilateral(const cv::Mat& frame, const std::vector<cv::Point>& quadrilateral,
                                   const cv::Scalar& goalLower, const cv::Scalar& goalUpper);
cv::Point findNearestFieldPoint(const std::vector<cv::Point>& quadrilateral, int bottomEdgeY, int frameHeight);
std::vector<cv::Point> detectQuadrilateral(const std::vector<cv::Point>& contour, 
                                          float epsilonFactor = 0.03f, 
                                          int minHeight = MIN_GOAL_HEIGHT, 
                                          int maxPoints = 6);
std::vector<cv::Point> orderQuadrilateralPoints(std::vector<cv::Point> pts);
std::vector<cv::Point> getReliableGoalpostContour(const std::vector<std::vector<cv::Point>>& contours, 
                                                 const cv::Mat& colorMask);
std::vector<std::vector<cv::Point>> filterOutRods(const std::vector<std::vector<cv::Point>>& contours);
std::vector<cv::Point> combineGoalpostParts(const std::vector<std::vector<cv::Point>>& contours, int minArea);
cv::Point findLineMidpoint(const std::vector<cv::Point>& linePoints);
cv::Point findQuadMidpoint(const std::vector<cv::Point>& quadPoints);
float findGoalAngle(const cv::Point& goalMidpoint);
std::vector<cv::Point> findClosestPointsToCenter(const std::vector<cv::Point>& quadPoints, 
                                               const cv::Point& centerPoint = CENTRE_POINT);

int main() {
    std::string videoPath = "vid4.mp4";
    std::cout << "File exists? " << (std::ifstream(videoPath).good() ? "yes" : "no") << std::endl;
    
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video file" << std::endl;
        return -1;
    }
    
    while (true) {
        cv::Mat frame;
        bool ret = cap.read(frame);
        if (!ret) break;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        cv::Mat hsv, output = frame.clone();
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        
        cv::Mat debugMask;
        if (DEBUG) {
            debugMask = cv::Mat::zeros(frame.size(), frame.type());
        }
        
        // Step 1: Create and clean color masks
        cv::Mat blueMask, yellowMask;
        cv::inRange(hsv, BLUE_LOWER, BLUE_UPPER, blueMask);
        cv::inRange(hsv, YELLOW_LOWER, YELLOW_UPPER, yellowMask);
        
        // Step 2: Apply morphology
        cv::Mat kernelSmall = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::morphologyEx(blueMask, blueMask, cv::MORPH_CLOSE, kernelSmall);
        cv::morphologyEx(yellowMask, yellowMask, cv::MORPH_CLOSE, kernelSmall);
        
        // Step 3: Find contours
        std::vector<std::vector<cv::Point>> blueContours, yellowContours;
        cv::findContours(blueMask, blueContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::findContours(yellowMask, yellowContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        // Step 4: Process blue goalpost
        bool blueDetected = false;
        std::vector<cv::Point> blueQuad;
        cv::circle(output, CENTRE_POINT, 5, cv::Scalar(255, 255, 255), -1);
        
        std::vector<std::vector<cv::Point>> filteredBlue = filterOutRods(blueContours);
        if (filteredBlue.size() > 1) {
            std::vector<cv::Point> combinedBlue = combineGoalpostParts(filteredBlue, MIN_CONTOUR_AREA / 2);
            if (!combinedBlue.empty()) {
                std::vector<cv::Point> quad = detectQuadrilateral(combinedBlue, EPSILON_FACTOR);
                if (!quad.empty()) {
                    blueDetected = true;
                    blueQuad = quad;
                }
            }
        }
        
        if (!blueDetected && !filteredBlue.empty()) {
            std::vector<cv::Point> reliableBlue = getReliableGoalpostContour(filteredBlue, blueMask);
            if (!reliableBlue.empty()) {
                std::vector<cv::Point> quad = detectQuadrilateral(reliableBlue, EPSILON_FACTOR);
                if (!quad.empty()) {
                    blueDetected = true;
                    blueQuad = quad;
                }
            }
        }
        
        if (!blueDetected && !blueContours.empty()) {
            auto largestBlue = *std::max_element(blueContours.begin(), blueContours.end(), 
                                               [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
                                                   return cv::contourArea(a) < cv::contourArea(b);
                                               });
            if (cv::contourArea(largestBlue) > MIN_CONTOUR_AREA) {
                std::vector<cv::Point> quad = detectQuadrilateral(largestBlue, EPSILON_FACTOR);
                if (!quad.empty()) {
                    blueDetected = true;
                    blueQuad = quad;
                }
            }
        }
        
        if (blueDetected) {
            // Draw the blue quadrilateral
            for (int i = 0; i < 4; i++) {
                cv::line(output, blueQuad[i], blueQuad[(i + 1) % 4], cv::Scalar(255, 0, 0), 2);
            }
            
            auto closestPoints = findClosestPointsToCenter(blueQuad);
            cv::Point point1 = closestPoints[0];
            cv::Point point2 = closestPoints[1];
            cv::Point blueBottomMidpoint = findLineMidpoint({point1, point2});
            
            cv::circle(output, point1, 7, cv::Scalar(255, 0, 0), -1);
            cv::circle(output, point2, 7, cv::Scalar(255, 0, 0), -1);
            cv::line(output, point1, point2, cv::Scalar(255, 0, 0), 2);
            
            int blueLineMidpointX = blueBottomMidpoint.x;
            int blueLineMidpointY = blueBottomMidpoint.y;
            cv::circle(output, cv::Point(blueLineMidpointX, blueLineMidpointY), 7, cv::Scalar(255, 255, 0), -1);
            
            cv::circle(output, point1, 7, cv::Scalar(0, 255, 255), -1);
            cv::circle(output, point2, 7, cv::Scalar(0, 255, 255), -1);
            cv::line(output, point1, point2, cv::Scalar(0, 255, 255), 2);
            
            std::cout << "Closest points to center: (" << point1.x << "," << point1.y << "), "
                      << "(" << point2.x << "," << point2.y << ")" << std::endl;
            
            int bottomEdgeY = findTrueBottomEdgeQuadrilateral(frame, blueQuad, BLUE_LOWER, BLUE_UPPER);
            cv::Point nearestPoint = findNearestFieldPoint(blueQuad, bottomEdgeY, frame.rows);
            int nearestX = nearestPoint.x;
            int nearestY = nearestPoint.y;
            cv::Point blueMidpoint = findQuadMidpoint(blueQuad);
            
            cv::circle(output, cv::Point(nearestX, nearestY), 5, cv::Scalar(255, 255, 0), -1);
            cv::circle(output, blueMidpoint, 5, cv::Scalar(255, 255, 255), -1);
            cv::line(output, cv::Point(nearestX - 10, nearestY), cv::Point(nearestX + 10, nearestY),
                    cv::Scalar(255, 255, 0), 2);
            
            cv::putText(output, "(" + std::to_string(nearestX) + "," + std::to_string(nearestY) + ")",
                       cv::Point(nearestX - 40, nearestY - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                       cv::Scalar(255, 255, 0), 1);
            
            cv::putText(output, "(" + std::to_string(blueMidpoint.x) + "," + std::to_string(blueMidpoint.y) + ")",
                       cv::Point(blueMidpoint.x - 40, blueMidpoint.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                       cv::Scalar(255, 255, 255), 1);
            
            float blueGoalAngle = findGoalAngle(blueMidpoint);
            cv::putText(output, "Angle: " + std::to_string(blueGoalAngle * 180 / M_PI).substr(0, 5) + " deg",
                       cv::Point(blueMidpoint.x - 40, blueMidpoint.y + 40), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                       cv::Scalar(0, 0, 255), 2);
        }
        
        // Step 5: Process yellow goalpost
        // (Similar processing as blue goalpost - abbreviated for brevity)
        bool yellowDetected = false;
        std::vector<cv::Point> yellowQuad;
        
        std::vector<std::vector<cv::Point>> filteredYellow = filterOutRods(yellowContours);
        // Process yellow goalpost similar to blue goalpost...
        
        // Display debug view
        if (DEBUG) {
            cv::Mat blueDisplay, yellowDisplay;
            cv::cvtColor(blueMask, blueDisplay, cv::COLOR_GRAY2BGR);
            cv::cvtColor(yellowMask, yellowDisplay, cv::COLOR_GRAY2BGR);
            
            cv::putText(blueDisplay, "Blue Mask", cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
            cv::putText(yellowDisplay, "Yellow Mask", cv::Point(10, 30), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
            
            int h = frame.rows, w = frame.cols;
            cv::resize(blueDisplay, blueDisplay, cv::Size(w / 2, h / 2));
            cv::resize(yellowDisplay, yellowDisplay, cv::Size(w / 2, h / 2));
            
            cv::Mat debugTop, debugView;
            cv::hconcat(blueDisplay, yellowDisplay, debugTop);
            
            cv::Mat resizedOutput;
            cv::resize(output, resizedOutput, cv::Size(w, h / 2));
            cv::vconcat(debugTop, resizedOutput, debugView);
            
            cv::imshow("Debug View", debugView);
        }
        
        cv::imshow("Goal Detection", output);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "Time taken for frame: " << duration.count() << " ms" << std::endl;
        
        if (cv::waitKey(25) == 'q') {
            break;
        }
    }
    
    cap.release();
    cv::destroyAllWindows();
    return 0;
}

// Helper function implementations
int findTrueBottomEdge(const cv::Mat& frame, const cv::Rect& bbox, 
                      const cv::Scalar& goalLower, const cv::Scalar& goalUpper) {
    int x = bbox.x;
    int y = bbox.y;
    int w = bbox.width;
    int h = bbox.height;
    
    int roiHeight = std::min(h + EDGE_SEARCH_HEIGHT, frame.rows - y);
    cv::Mat roi = frame(cv::Rect(x, y, w, roiHeight));
    
    cv::Mat hsv, goalMask, fieldMask;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, goalLower, goalUpper, goalMask);
    cv::inRange(hsv, FIELD_LOWER, FIELD_UPPER, fieldMask);
    
    // Calculate vertical projection
    std::vector<int> verticalProj(roiHeight, 0);
    for (int i = 0; i < roiHeight; i++) {
        verticalProj[i] = cv::sum(goalMask.row(i))[0];
    }
    
    int maxVal = *std::max_element(verticalProj.begin(), verticalProj.end());
    int threshold = 0.4 * maxVal;
    
    // Find goal rows
    std::vector<int> goalRows;
    for (int i = 0; i < verticalProj.size(); i++) {
        if (verticalProj[i] > threshold) {
            goalRows.push_back(i);
        }
    }
    
    if (goalRows.empty()) {
        return y + h;
    }
    
    int bottomRow = goalRows.back();
    for (int row = bottomRow; row < std::min(bottomRow + EDGE_SEARCH_HEIGHT, roiHeight); row++) {
        if (row >= fieldMask.rows) {
            break;
        }
        if (cv::sum(fieldMask.row(row))[0] > 0.1 * w * 255) {
            bottomRow = row;
            break;
        }
    }
    
    return y + bottomRow;
}

int findTrueBottomEdgeQuadrilateral(const cv::Mat& frame, const std::vector<cv::Point>& quadrilateral,
                                   const cv::Scalar& goalLower, const cv::Scalar& goalUpper) {
    cv::Rect bbox = cv::boundingRect(quadrilateral);
    int x = bbox.x;
    int y = bbox.y;
    int w = bbox.width;
    int h = bbox.height;
    
    int roiHeight = std::min(h + EDGE_SEARCH_HEIGHT, frame.rows - y);
    cv::Mat roi = frame(cv::Rect(x, y, w, roiHeight));
    
    cv::Mat mask = cv::Mat::zeros(roiHeight, w, CV_8UC1);
    std::vector<cv::Point> shiftedQuad;
    for (const auto& pt : quadrilateral) {
        shiftedQuad.push_back(cv::Point(pt.x - x, pt.y - y));
    }
    
    std::vector<std::vector<cv::Point>> contours{shiftedQuad};
    cv::fillPoly(mask, contours, 255);
    
    cv::Mat hsv, goalMask, fieldMask;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, goalLower, goalUpper, goalMask);
    cv::inRange(hsv, FIELD_LOWER, FIELD_UPPER, fieldMask);
    
    goalMask = goalMask & mask;
    fieldMask = fieldMask & mask;
    
    // Calculate vertical projection
    std::vector<int> verticalProj(roiHeight, 0);
    for (int i = 0; i < roiHeight; i++) {
        verticalProj[i] = cv::sum(goalMask.row(i))[0];
    }
    
    int maxVal = 0;
    for (int val : verticalProj) {
        maxVal = std::max(maxVal, val);
    }
    
    int threshold = 0.2 * maxVal;
    
    // Find goal rows
    std::vector<int> goalRows;
    for (int i = 0; i < verticalProj.size(); i++) {
        if (verticalProj[i] > threshold) {
            goalRows.push_back(i);
        }
    }
    
    if (goalRows.empty()) {
        return y + h;
    }
    
    int bottomRow = goalRows.back();
    for (int row = bottomRow; row < std::min(bottomRow + EDGE_SEARCH_HEIGHT, roiHeight); row++) {
        if (row >= fieldMask.rows) {
            break;
        }
        if (cv::sum(fieldMask.row(row))[0] > 0.05 * w * 255) {
            bottomRow = row;
            break;
        }
    }
    
    return y + bottomRow;
}

cv::Point findNearestFieldPoint(const std::vector<cv::Point>& quadrilateral, int bottomEdgeY, int frameHeight) {
    int botCenterX = frameHeight / 2;
    int botCenterY = frameHeight;
    int fieldThreshold = 20;
    std::vector<cv::Point> fieldPoints;
    
    for (const auto& point : quadrilateral) {
        if (std::abs(point.y - bottomEdgeY) < fieldThreshold) {
            fieldPoints.push_back(point);
        }
    }
    
    if (fieldPoints.empty()) {
        fieldPoints = quadrilateral;
    }
    
    cv::Point closestPoint;
    double minDistance = std::numeric_limits<double>::infinity();
    
    for (const auto& point : fieldPoints) {
        double distance = std::sqrt(
            std::pow(point.x - botCenterX, 2) + 
            std::pow(point.y - botCenterY, 2)
        );
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = point;
        }
    }
    
    return closestPoint;
}

std::vector<cv::Point> detectQuadrilateral(const std::vector<cv::Point>& contour, 
                                          float epsilonFactor, int minHeight, int maxPoints) {
    double perimeter = cv::arcLength(contour, true);
    double epsilon = epsilonFactor * perimeter;
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, epsilon, true);
    
    if (DEBUG) {
        std::cout << "Contour has " << approx.size() << " points" << std::endl;
    }
    
    if (approx.size() < 4 || approx.size() > maxPoints) {
        if (DEBUG) {
            std::cout << "Skipping shape with " << approx.size() << " points" << std::endl;
        }
        return {};
    }
    
    std::vector<cv::Point> pts;
    if (approx.size() > 4) {
        // Find center
        cv::Point center(0, 0);
        for (const auto& pt : approx) {
            center.x += pt.x;
            center.y += pt.y;
        }
        center.x /= approx.size();
        center.y /= approx.size();
        
        // Calculate distances from center
        std::vector<std::pair<int, double>> distances;
        for (int i = 0; i < approx.size(); i++) {
            double dist = std::sqrt(
                std::pow(approx[i].x - center.x, 2) + 
                std::pow(approx[i].y - center.y, 2)
            );
            distances.push_back({i, dist});
        }
        
        // Sort by distance and get furthest 4 points
        std::sort(distances.begin(), distances.end(), 
                 [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                     return a.second > b.second;
                 });
        
        for (int i = 0; i < 4; i++) {
            pts.push_back(approx[distances[i].first]);
        }
    } else {
        pts = approx;
    }
    
    cv::Rect bbox = cv::boundingRect(pts);
    if (bbox.height < minHeight || bbox.width < 5 || 
        static_cast<float>(bbox.height) / bbox.width > 8.0f) {
        if (DEBUG) {
            std::cout << "Skipping shape with h/w ratio " 
                     << static_cast<float>(bbox.height) / bbox.width << std::endl;
        }
        return {};
    }
    
    if (pts.size() == 4) {
        pts = orderQuadrilateralPoints(pts);
    }
    
    return pts;
}

std::vector<cv::Point> orderQuadrilateralPoints(std::vector<cv::Point> pts) {
    // Sort points by x-coordinate
    std::sort(pts.begin(), pts.end(), [](const cv::Point& a, const cv::Point& b) {
        return a.x < b.x;
    });
    
    std::vector<cv::Point> leftPoints = {pts[0], pts[1]};
    std::vector<cv::Point> rightPoints = {pts[2], pts[3]};
    
    // Sort left points by y-coordinate
    std::sort(leftPoints.begin(), leftPoints.end(), [](const cv::Point& a, const cv::Point& b) {
        return a.y < b.y;
    });
    
    // Sort right points by y-coordinate
    std::sort(rightPoints.begin(), rightPoints.end(), [](const cv::Point& a, const cv::Point& b) {
        return a.y < b.y;
    });
    
    return {
        leftPoints[0],   // top-left
        rightPoints[0],  // top-right
        rightPoints[1],  // bottom-right
        leftPoints[1]    // bottom-left
    };
}

std::vector<cv::Point> getReliableGoalpostContour(const std::vector<std::vector<cv::Point>>& contours, 
                                                const cv::Mat& colorMask) {
    if (contours.empty()) {
        return {};
    }
    
    std::vector<std::vector<cv::Point>> sortedContours = contours;
    std::sort(sortedContours.begin(), sortedContours.end(), 
             [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
                 return cv::contourArea(a) > cv::contourArea(b);
             });
    
    std::vector<cv::Point> largest = sortedContours[0];
    if (cv::contourArea(largest) < MIN_CONTOUR_AREA) {
        return {};
    }
    
    std::vector<cv::Point> hull;
    cv::convexHull(largest, hull);
    
    cv::Rect bbox = cv::boundingRect(hull);
    float aspectRatio = static_cast<float>(bbox.height) / std::max(bbox.width, 1);
    
    if (aspectRatio > 1.5f) {
        return hull;
    }
    return largest;
}

std::vector<std::vector<cv::Point>> filterOutRods(const std::vector<std::vector<cv::Point>>& contours) {
    std::vector<std::vector<cv::Point>> filteredContours;
    for (const auto& contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float width = rect.size.width;
        float height = rect.size.height;
        
        if (width > height) {
            std::swap(width, height);
        }
        
        float aspectRatio = height / std::max(width, 1.0f);
        if (aspectRatio > 8.0f) {
            continue;
        }
        
        filteredContours.push_back(contour);
    }
    
    return filteredContours;
}

std::vector<cv::Point> combineGoalpostParts(const std::vector<std::vector<cv::Point>>& contours, int minArea) {
    std::vector<std::vector<cv::Point>> significantContours;
    for (const auto& c : contours) {
        if (cv::contourArea(c) > minArea / 4) {
            significantContours.push_back(c);
        }
    }
    
    if (significantContours.size() > 1) {
        std::vector<cv::Point> allPoints;
        for (const auto& contour : significantContours) {
            allPoints.insert(allPoints.end(), contour.begin(), contour.end());
        }
        
        std::vector<cv::Point> hull;
        cv::convexHull(allPoints, hull);
        return hull;
    } else if (significantContours.size() == 1) {
        return significantContours[0];
    }
    
    return {};
}

cv::Point findLineMidpoint(const std::vector<cv::Point>& linePoints) {
    if (linePoints.size() != 2) {
        return cv::Point(0, 0);
    }
    
    int totalX = (linePoints[0].x + linePoints[1].x) / 2;
    int totalY = (linePoints[0].y + linePoints[1].y) / 2;
    return cv::Point(totalX, totalY);
}

cv::Point findQuadMidpoint(const std::vector<cv::Point>& quadPoints) {
    if (quadPoints.size() != 4) {
        return cv::Point(0, 0);
    }
    
    int totalX = 0;
    int totalY = 0;
    for (int i = 0; i < 4; i++) {
        totalX += quadPoints[i].x;
        totalY += quadPoints[i].y;
    }
    return cv::Point(totalX / 4, totalY / 4);
}

float findGoalAngle(const cv::Point& goalMidpoint) {
    int deltaX = goalMidpoint.x - CENTRE_POINT.x;
    int deltaY = goalMidpoint.y - CENTRE_POINT.y;
    float goalAngle = std::atan2(deltaY, deltaX);
    return goalAngle + M_PI / 2;
}

std::vector<cv::Point> findClosestPointsToCenter(const std::vector<cv::Point>& quadPoints, 
                                               const cv::Point& centerPoint) {
    std::vector<std::pair<int, double>> distances;
    for (int i = 0; i < quadPoints.size(); i++) {
        double dist = std::sqrt(
            std::pow(quadPoints[i].x - centerPoint.x, 2) + 
            std::pow(quadPoints[i].y - centerPoint.y, 2)
        );
        distances.push_back({i, dist});
    }
    
    std::sort(distances.begin(), distances.end(), 
             [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                 return a.second < b.second;
             });
    
    std::vector<cv::Point> closestPoints = {
        quadPoints[distances[0].first],
        quadPoints[distances[1].first]
    };
    
    return closestPoints;
}