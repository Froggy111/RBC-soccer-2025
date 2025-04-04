#include "goalpost_detection.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

GoalpostDetector::GoalpostDetector(const std::string& videoPath, bool debug) :
    DEBUG(debug),
    videoPath(videoPath),
    // Initialize constants
    MIN_CONTOUR_AREA(400),
    MIN_GOAL_HEIGHT(10),
    EDGE_SEARCH_HEIGHT(69),
    EPSILON_FACTOR(0.05),
    PARALLELOGRAM_TOLERANCE(1),
    CENTRE_POINT(640 / 2 - 5, 480 / 2 + 30)
{
    // Initialize HSV color ranges
    BLUE_LOWER = cv::Scalar(100, 50, 50);    // Blue around 100-130 hue
    BLUE_UPPER = cv::Scalar(150, 255, 255);  // Upper limit within valid range
    
    YELLOW_LOWER = cv::Scalar(20, 137, 110); // Yellow is around 20-30 in OpenCV HSV
    YELLOW_UPPER = cv::Scalar(30, 255, 255);
    
    FIELD_LOWER = cv::Scalar(35, 50, 50);    // Green is around 35-85 in OpenCV HSV
    FIELD_UPPER = cv::Scalar(85, 255, 255);
}

GoalpostDetector::~GoalpostDetector() {
    // Release resources
    if (cap.isOpened()) {
        cap.release();
    }
    cv::destroyAllWindows();
}

bool GoalpostDetector::initialize() {
    // Check if file exists
    std::ifstream file(videoPath);
    bool fileExists = file.good();
    file.close();
    
    std::cout << "file exists? " << (fileExists ? "true" : "false") << std::endl;
    
    // Open video capture
    cap.open(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file: " << videoPath << std::endl;
        return false;
    }
    
    return true;
}

int GoalpostDetector::findTrueBottomEdge(const cv::Mat& frame, const cv::Rect& bbox, 
                                        const cv::Scalar& goalLower, const cv::Scalar& goalUpper) {
    int x = bbox.x;
    int y = bbox.y;
    int w = bbox.width;
    int h = bbox.height;
    
    int roiHeight = std::min(h + EDGE_SEARCH_HEIGHT, frame.rows - y);
    cv::Mat roi = frame(cv::Rect(x, y, w, roiHeight));
    
    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    
    cv::Mat goalMask, fieldMask;
    cv::inRange(hsv, goalLower, goalUpper, goalMask);
    cv::inRange(hsv, FIELD_LOWER, FIELD_UPPER, fieldMask);
    
    // Calculate vertical projection (sum along rows)
    std::vector<int> verticalProj(goalMask.rows, 0);
    for (int r = 0; r < goalMask.rows; r++) {
        verticalProj[r] = cv::sum(goalMask.row(r))[0];
    }
    
    // Find maximum of vertical projection
    int maxProj = 0;
    for (int val : verticalProj) {
        maxProj = std::max(maxProj, val);
    }
    
    int threshold = 0.4 * maxProj;
    
    // Find rows where projection is greater than threshold
    std::vector<int> goalRows;
    for (int r = 0; r < verticalProj.size(); r++) {
        if (verticalProj[r] > threshold) {
            goalRows.push_back(r);
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

int GoalpostDetector::findTrueBottomEdgeParallelogram(const cv::Mat& frame, const cv::Mat& parallelogram, 
                                                    const cv::Scalar& goalLower, const cv::Scalar& goalUpper) {
    cv::Rect bbox = cv::boundingRect(parallelogram);
    int x = bbox.x;
    int y = bbox.y;
    int w = bbox.width;
    int h = bbox.height;
    
    int roiHeight = std::min(h + EDGE_SEARCH_HEIGHT, frame.rows - y);
    cv::Mat roi = frame(cv::Rect(x, y, w, roiHeight));
    
    // Create mask for the parallelogram
    cv::Mat mask = cv::Mat::zeros(roiHeight, w, CV_8UC1);
    
    // Shift parallelogram to ROI coordinates
    std::vector<cv::Point> shiftedPoly;
    for (int i = 0; i < parallelogram.rows; i++) {
        cv::Point p(parallelogram.at<int>(i, 0) - x, parallelogram.at<int>(i, 1) - y);
        shiftedPoly.push_back(p);
    }
    
    std::vector<std::vector<cv::Point>> contours = {shiftedPoly};
    cv::fillPoly(mask, contours, 255);
    
    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    
    cv::Mat goalMask, fieldMask;
    cv::inRange(hsv, goalLower, goalUpper, goalMask);
    cv::inRange(hsv, FIELD_LOWER, FIELD_UPPER, fieldMask);
    
    // Apply the parallelogram mask
    cv::bitwise_and(goalMask, mask, goalMask);
    cv::bitwise_and(fieldMask, mask, fieldMask);
    
    // Calculate vertical projection
    std::vector<int> verticalProj(goalMask.rows, 0);
    for (int r = 0; r < goalMask.rows; r++) {
        verticalProj[r] = cv::sum(goalMask.row(r))[0];
    }
    
    // Find maximum of vertical projection
    int maxProj = 0;
    for (int val : verticalProj) {
        maxProj = std::max(maxProj, val);
    }
    
    int threshold = 0.4 * maxProj;
    
    // Find rows where projection is greater than threshold
    std::vector<int> goalRows;
    for (int r = 0; r < verticalProj.size(); r++) {
        if (verticalProj[r] > threshold) {
            goalRows.push_back(r);
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

int GoalpostDetector::findTrueBottomEdgeQuadrilateral(const cv::Mat& frame, const cv::Mat& quadrilateral, 
                                                     const cv::Scalar& goalLower, const cv::Scalar& goalUpper) {
    // Similar to findTrueBottomEdgeParallelogram but adapted for quadrilateral
    cv::Rect bbox = cv::boundingRect(quadrilateral);
    int x = bbox.x;
    int y = bbox.y;
    int w = bbox.width;
    int h = bbox.height;
    
    int roiHeight = std::min(h + EDGE_SEARCH_HEIGHT, frame.rows - y);
    cv::Mat roi = frame(cv::Rect(x, y, w, roiHeight));
    
    // Create mask for the quadrilateral
    cv::Mat mask = cv::Mat::zeros(roiHeight, w, CV_8UC1);
    
    // Shift quadrilateral to ROI coordinates and clip to valid range
    std::vector<cv::Point> shiftedQuad;
    for (int i = 0; i < quadrilateral.rows; i++) {
        int px = std::max(0, std::min(w - 1, quadrilateral.at<int>(i, 0) - x));
        int py = std::max(0, std::min(roiHeight - 1, quadrilateral.at<int>(i, 1) - y));
        shiftedQuad.push_back(cv::Point(px, py));
    }
    
    std::vector<std::vector<cv::Point>> contours = {shiftedQuad};
    cv::fillPoly(mask, contours, 255);
    
    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    
    cv::Mat goalMask, fieldMask;
    cv::inRange(hsv, goalLower, goalUpper, goalMask);
    cv::inRange(hsv, FIELD_LOWER, FIELD_UPPER, fieldMask);
    
    // Apply the quadrilateral mask
    cv::bitwise_and(goalMask, mask, goalMask);
    cv::bitwise_and(fieldMask, mask, fieldMask);
    
    // Calculate vertical projection
    std::vector<int> verticalProj(goalMask.rows, 0);
    for (int r = 0; r < goalMask.rows; r++) {
        verticalProj[r] = cv::sum(goalMask.row(r))[0];
    }
    
    // Find maximum of vertical projection
    int maxProj = 0;
    for (int val : verticalProj) {
        maxProj = std::max(maxProj, val);
    }
    
    int threshold = 0.2 * maxProj;
    
    // Find rows where projection is greater than threshold
    std::vector<int> goalRows;
    for (int r = 0; r < verticalProj.size(); r++) {
        if (verticalProj[r] > threshold) {
            goalRows.push_back(r);
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

cv::Mat GoalpostDetector::orderPoints(const cv::Mat& pts) {
    cv::Mat rect = cv::Mat::zeros(4, 2, CV_32F);
    
    // Compute sums and differences
    std::vector<float> sums(pts.rows);
    std::vector<float> diffs(pts.rows);
    
    for (int i = 0; i < pts.rows; i++) {
        sums[i] = pts.at<float>(i, 0) + pts.at<float>(i, 1);
        diffs[i] = pts.at<float>(i, 1) - pts.at<float>(i, 0);
    }
    
    // Find indices
    int minSumIdx = std::min_element(sums.begin(), sums.end()) - sums.begin();
    int maxSumIdx = std::max_element(sums.begin(), sums.end()) - sums.begin();
    int minDiffIdx = std::min_element(diffs.begin(), diffs.end()) - diffs.begin();
    int maxDiffIdx = std::max_element(diffs.begin(), diffs.end()) - diffs.begin();
    
    // Assign points in order [top-left, top-right, bottom-right, bottom-left]
    pts.row(minSumIdx).copyTo(rect.row(0));
    pts.row(minDiffIdx).copyTo(rect.row(1));
    pts.row(maxSumIdx).copyTo(rect.row(2));
    pts.row(maxDiffIdx).copyTo(rect.row(3));
    
    return rect;
}

cv::Mat GoalpostDetector::detectQuadrilateral(const std::vector<cv::Point>& contour, double epsilonFactor,
                                            int minHeight, int maxPoints) {
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
        return cv::Mat();
    }
    
    cv::Mat pts;
    if (approx.size() > 4) {
        // Find center
        cv::Point2f center(0, 0);
        for (const auto& p : approx) {
            center.x += p.x;
            center.y += p.y;
        }
        center.x /= approx.size();
        center.y /= approx.size();
        
        // Calculate distances from center
        std::vector<std::pair<int, float>> distances;
        for (int i = 0; i < approx.size(); i++) {
            float dist = std::sqrt(std::pow(approx[i].x - center.x, 2) + 
                                  std::pow(approx[i].y - center.y, 2));
            distances.push_back({i, dist});
        }
        
        // Sort by distance (descending)
        std::sort(distances.begin(), distances.end(), 
                 [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                     return a.second > b.second;
                 });
        
        // Take 4 furthest points
        pts = cv::Mat(4, 2, CV_32F);
        for (int i = 0; i < 4; i++) {
            int idx = distances[i].first;
            pts.at<float>(i, 0) = static_cast<float>(approx[idx].x);
            pts.at<float>(i, 1) = static_cast<float>(approx[idx].y);
        }
    } else {
        // If exactly 4 points, convert to matrix
        pts = cv::Mat(4, 2, CV_32F);
        for (int i = 0; i < 4; i++) {
            pts.at<float>(i, 0) = static_cast<float>(approx[i].x);
            pts.at<float>(i, 1) = static_cast<float>(approx[i].y);
        }
    }
    
    // Check dimensions
    cv::Rect bbox = cv::boundingRect(approx);
    if (bbox.height < minHeight || bbox.width < 5 || 
        (static_cast<float>(bbox.height) / bbox.width) > 8.0) {
        if (DEBUG) {
            std::cout << "Skipping shape with h/w ratio " 
                     << (static_cast<float>(bbox.height) / bbox.width) << std::endl;
        }
        return cv::Mat();
    }
    
    // Order points
    if (pts.rows == 4) {
        pts = orderQuadrilateralPoints(pts);
    }
    
    return pts;
}

cv::Mat GoalpostDetector::orderQuadrilateralPoints(const cv::Mat& pts) {
    // Create output matrix
    cv::Mat ordered = cv::Mat(4, 2, CV_32F);
    
    // Sort by x-coordinate
    std::vector<std::pair<int, float>> xSorted;
    for (int i = 0; i < pts.rows; i++) {
        xSorted.push_back({i, pts.at<float>(i, 0)});
    }
    
    std::sort(xSorted.begin(), xSorted.end(), 
             [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                 return a.second < b.second;
             });
    
    // Get left and right points
    std::vector<std::pair<int, float>> leftPoints = {xSorted[0], xSorted[1]};
    std::vector<std::pair<int, float>> rightPoints = {xSorted[2], xSorted[3]};
    
    // Sort left and right points by y-coordinate
    std::sort(leftPoints.begin(), leftPoints.end(), 
             [&pts](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                 return pts.at<float>(a.first, 1) < pts.at<float>(b.first, 1);
             });
    
    std::sort(rightPoints.begin(), rightPoints.end(), 
             [&pts](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                 return pts.at<float>(a.first, 1) < pts.at<float>(b.first, 1);
             });
    
    // Assign to result matrix (top-left, top-right, bottom-right, bottom-left)
    int idx = leftPoints[0].first;
    ordered.at<float>(0, 0) = pts.at<float>(idx, 0);
    ordered.at<float>(0, 1) = pts.at<float>(idx, 1);
    
    idx = rightPoints[0].first;
    ordered.at<float>(1, 0) = pts.at<float>(idx, 0);
    ordered.at<float>(1, 1) = pts.at<float>(idx, 1);
    
    idx = rightPoints[1].first;
    ordered.at<float>(2, 0) = pts.at<float>(idx, 0);
    ordered.at<float>(2, 1) = pts.at<float>(idx, 1);
    
    idx = leftPoints[1].first;
    ordered.at<float>(3, 0) = pts.at<float>(idx, 0);
    ordered.at<float>(3, 1) = pts.at<float>(idx, 1);
    
    return ordered;
}

cv::Point GoalpostDetector::findNearestFieldPoint(const cv::Mat& quadrilateral, int bottomEdgeY, int frameHeight) {
    int botCenterX = frameHeight / 2;
    int botCenterY = frameHeight;
    int fieldThreshold = 20;
    std::vector<cv::Point> fieldPoints;
    
    // Find points near the bottom edge
    for (int i = 0; i < quadrilateral.rows; i++) {
        int pointY = static_cast<int>(quadrilateral.at<float>(i, 1));
        if (std::abs(pointY - bottomEdgeY) < fieldThreshold) {
            fieldPoints.push_back(cv::Point(
                static_cast<int>(quadrilateral.at<float>(i, 0)),
                pointY
            ));
        }
    }
    
    // If no field points were found, use all points
    if (fieldPoints.empty()) {
        for (int i = 0; i < quadrilateral.rows; i++) {
            fieldPoints.push_back(cv::Point(
                static_cast<int>(quadrilateral.at<float>(i, 0)),
                static_cast<int>(quadrilateral.at<float>(i, 1))
            ));
        }
    }
    
    // Find closest point to robot center
    cv::Point closestPoint;
    double minDistance = std::numeric_limits<double>::max();
    
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

std::vector<cv::Point> GoalpostDetector::getReliableGoalpostContour(
    const std::vector<std::vector<cv::Point>>& contours, 
    const cv::Mat& colorMask) {
    
    if (contours.empty()) {
        return std::vector<cv::Point>();
    }
    
    // Sort contours by area (descending)
    std::vector<std::pair<int, double>> sortedIndices;
    for (int i = 0; i < contours.size(); i++) {
        sortedIndices.push_back({i, cv::contourArea(contours[i])});
    }
    
    std::sort(sortedIndices.begin(), sortedIndices.end(), 
             [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                 return a.second > b.second;
             });
    
    int largestIdx = sortedIndices[0].first;
    if (sortedIndices[0].second < MIN_CONTOUR_AREA) {
        return std::vector<cv::Point>();
    }
    
    std::vector<cv::Point> hull;
    cv::convexHull(contours[largestIdx], hull);
    
    cv::Rect bbox = cv::boundingRect(hull);
    double aspectRatio = static_cast<double>(bbox.height) / std::max(bbox.width, 1);
    
    if (aspectRatio > 1.5) {
        return hull;
    }
    
    return contours[largestIdx];
}

std::vector<std::vector<cv::Point>> GoalpostDetector::filterOutRods(
    const std::vector<std::vector<cv::Point>>& contours) {
    
    std::vector<std::vector<cv::Point>> filteredContours;
    
    for (const auto& contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float width = rect.size.width;
        float height = rect.size.height;
        
        if (width > height) {
            std::swap(width, height);
        }
        
        float aspectRatio = height / std::max(width, 1.0f);
        if (aspectRatio > 8.0) {
            continue;
        }
        
        filteredContours.push_back(contour);
    }
    
    return filteredContours;
}

std::vector<cv::Point> GoalpostDetector::combineGoalpostParts(
    const std::vector<std::vector<cv::Point>>& contours, int minArea) {
    
    std::vector<std::vector<cv::Point>> significantContours;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) > minArea / 4) {
            significantContours.push_back(contour);
        }
    }
    
    if (significantContours.size() > 1) {
        // Combine all points
        std::vector<cv::Point> allPoints;
        for (const auto& contour : significantContours) {
            allPoints.insert(allPoints.end(), contour.begin(), contour.end());
        }
        
        // Create convex hull
        std::vector<cv::Point> hull;
        cv::convexHull(allPoints, hull);
        return hull;
    }
    else if (significantContours.size() == 1) {
        return significantContours[0];
    }
    
    return std::vector<cv::Point>();
}

cv::Point2f GoalpostDetector::findLineMidpoint(const std::vector<cv::Point>& linePoints) {
    if (linePoints.size() != 2) {
        return cv::Point2f(0, 0);
    }
    
    float totalX = (linePoints[0].x + linePoints[1].x) / 2.0f;
    float totalY = (linePoints[0].y + linePoints[1].y) / 2.0f;
    
    return cv::Point2f(totalX, totalY);
}

cv::Point2f GoalpostDetector::findQuadMidpoint(const cv::Mat& quadPoints) {
    float totalX = 0.0f;
    float totalY = 0.0f;
    
    for (int i = 0; i < quadPoints.rows; i++) {
        totalX += quadPoints.at<float>(i, 0);
        totalY += quadPoints.at<float>(i, 1);
    }
    
    return cv::Point2f(totalX / quadPoints.rows, totalY / quadPoints.rows);
}

double GoalpostDetector::findGoalAngle(const cv::Point2f& goalMidpoint) {
    float deltaX = goalMidpoint.x - CENTRE_POINT.x;
    float deltaY = goalMidpoint.y - CENTRE_POINT.y;
    
    double goalAngle = std::atan2(deltaY, deltaX);
    return goalAngle + M_PI / 2.0;
}

std::vector<cv::Point> GoalpostDetector::findClosestPointsToCenter(
    const cv::Mat& quadPoints, const cv::Point& centerPoint) {
    
    std::vector<std::pair<int, double>> distances;
    cv::Point actualCenter = centerPoint;
    
    if (centerPoint.x == 0 && centerPoint.y == 0) {
        actualCenter = CENTRE_POINT;
    }
    
    for (int i = 0; i < quadPoints.rows; i++) {
        int px = static_cast<int>(quadPoints.at<float>(i, 0));
        int py = static_cast<int>(quadPoints.at<float>(i, 1));
        
        double dist = std::sqrt(
            std::pow(px - actualCenter.x, 2) + 
            std::pow(py - actualCenter.y, 2)
        );
        
        distances.push_back({i, dist});
    }
    
    std::sort(distances.begin(), distances.end(), 
             [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                 return a.second < b.second;
             });
    
    std::vector<cv::Point> closestPoints = {
        cv::Point(
            static_cast<int>(quadPoints.at<float>(distances[0].first, 0)),
            static_cast<int>(quadPoints.at<float>(distances[0].first, 1))
        ),
        cv::Point(
            static_cast<int>(quadPoints.at<float>(distances[1].first, 0)),
            static_cast<int>(quadPoints.at<float>(distances[1].first, 1))
        )
    };
    
    return closestPoints;
}

double GoalpostDetector::findDistanceToGoal(const cv::Point2f& goalMidpoint) {
    float xDelta = goalMidpoint.x - CENTRE_POINT.x;
    float yDelta = goalMidpoint.y - CENTRE_POINT.y;
    
    return std::sqrt(xDelta * xDelta + yDelta * yDelta);
}

cv::Mat GoalpostDetector::processFrame(cv::Mat& frame, bool& blueDetected, bool& yellowDetected,
                                      double& blueGoalAngle, double& yellowGoalAngle,
                                      double& blueGoalDistance, double& yellowGoalDistance) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process frame
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::Mat output = frame.clone();
    
    cv::Mat debugMask;
    if (DEBUG) {
        debugMask = cv::Mat::zeros(frame.size(), frame.type());
    }
    
    // Step 1: Create and clean color masks
    cv::Mat blueMask, yellowMask;
    cv::inRange(hsv, BLUE_LOWER, BLUE_UPPER, blueMask);
    cv::inRange(hsv, YELLOW_LOWER, YELLOW_UPPER, yellowMask);
    
    // Step 2: Apply morphology (minimal)
    cv::Mat kernelSmall = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(blueMask, blueMask, cv::MORPH_CLOSE, kernelSmall);
    cv::morphologyEx(yellowMask, yellowMask, cv::MORPH_CLOSE, kernelSmall);
    
    // Step 3: Find contours
    std::vector<std::vector<cv::Point>> blueContours, yellowContours;
    std::vector<cv::Vec4i> hierarchy;
    
    cv::findContours(blueMask.clone(), blueContours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(yellowMask.clone(), yellowContours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Step 4: Process blue goalpost
    blueDetected = false;
    cv::circle(output, CENTRE_POINT, 5, cv::Scalar(255, 255, 255), -1);
    
    // Draw center point
    cv::Mat blueQuad;
    
    std::vector<std::vector<cv::Point>> filteredBlue = filterOutRods(blueContours);
    if (filteredBlue.size() > 1) {
        std::vector<cv::Point> combinedBlue = combineGoalpostParts(filteredBlue, MIN_CONTOUR_AREA / 2);
        if (!combinedBlue.empty()) {
            blueQuad = detectQuadrilateral(combinedBlue, EPSILON_FACTOR);
            if (!blueQuad.empty()) {
                blueDetected = true;
            }
        }
    }
    
    if (!blueDetected && !filteredBlue.empty()) {
        std::vector<cv::Point> reliableBlue = getReliableGoalpostContour(filteredBlue, blueMask);
        if (!reliableBlue.empty()) {
            blueQuad = detectQuadrilateral(reliableBlue, EPSILON_FACTOR);
            if (!blueQuad.empty()) {
                blueDetected = true;
            }
        }
    }
    
    if (!blueDetected && !blueContours.empty()) {
        // Find largest contour
        int largestIdx = 0;
        double largestArea = 0;
        
        for (int i = 0; i < blueContours.size(); i++) {
            double area = cv::contourArea(blueContours[i]);
            if (area > largestArea) {
                largestArea = area;
                largestIdx = i;
            }
        }
        
        if (largestArea > MIN_CONTOUR_AREA) {
            blueQuad = detectQuadrilateral(blueContours[largestIdx], EPSILON_FACTOR);
            if (!blueQuad.empty()) {
                blueDetected = true;
            }
        }
    }
    
    // Process blue quadrilateral if detected
    if (blueDetected) {
        // Draw quadrilateral
        for (int i = 0; i < 4; i++) {
            cv::Point pt1(static_cast<int>(blueQuad.at<float>(i, 0)), 
                        static_cast<int>(blueQuad.at<float>(i, 1)));
            cv::Point pt2(static_cast<int>(blueQuad.at<float>((i + 1) % 4, 0)), 
                        static_cast<int>(blueQuad.at<float>((i + 1) % 4, 1)));
            cv::line(output, pt1, pt2, cv::Scalar(255, 0, 0), 2);
        }
        
        // Find closest points to center
        std::vector<cv::Point> closestPoints = findClosestPointsToCenter(blueQuad);
        cv::Point point1 = closestPoints[0];
        cv::Point point2 = closestPoints[1];
        
        cv::Point2f blueBottomMidpoint = findLineMidpoint({point1, point2});
        blueGoalDistance = findDistanceToGoal(blueBottomMidpoint);
        std::cout << "Distance to Blue Goal: " << blueGoalDistance << std::endl;
        
        cv::circle(output, point1, 7, cv::Scalar(255, 0, 0), -1);
        cv::circle(output, point2, 7, cv::Scalar(255, 0, 0), -1);
        cv::line(output, point1, point2, cv::Scalar(255, 0, 0), 2);
        
        int blueMidpointX = static_cast<int>(blueBottomMidpoint.x);
        int blueMidpointY = static_cast<int>(blueBottomMidpoint.y);
        cv::circle(output, cv::Point(blueMidpointX, blueMidpointY), 7, cv::Scalar(255, 255, 0), -1);
        
        cv::circle(output, point1, 7, cv::Scalar(0, 255, 255), -1);
        cv::circle(output, point2, 7, cv::Scalar(0, 255, 255), -1);
        cv::line(output, point1, point2, cv::Scalar(0, 255, 255), 2);
        
        std::cout << "Closest points to center: " << point1 << ", " << point2 << std::endl;
        
        // Find bottom edge and nearest field point
        int bottomEdgeY = findTrueBottomEdgeQuadrilateral(frame, blueQuad, BLUE_LOWER, BLUE_UPPER);
        cv::Point nearestPoint = findNearestFieldPoint(blueQuad, bottomEdgeY, frame.rows);
        
        cv::Point2f blueMidpoint = findQuadMidpoint(blueQuad);
        std::cout << blueMidpoint << std::endl;
        
        cv::circle(output, nearestPoint, 5, cv::Scalar(255, 255, 0), -1);
        cv::circle(output, cv::Point(static_cast<int>(blueMidpoint.x), static_cast<int>(blueMidpoint.y)), 
                  5, cv::Scalar(255, 255, 255), -1);
                  
        cv::line(output, cv::Point(nearestPoint.x - 10, nearestPoint.y), 
                cv::Point(nearestPoint.x + 10, nearestPoint.y), cv::Scalar(255, 255, 0), 2);
                
        // Draw text info
        cv::putText(output, "(" + std::to_string(nearestPoint.x) + "," + std::to_string(nearestPoint.y) + ")",
                   cv::Point(nearestPoint.x - 40, nearestPoint.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
                   
        cv::putText(output, "(" + std::to_string(static_cast<int>(blueMidpoint.x)) + "," + 
                   std::to_string(static_cast<int>(blueMidpoint.y)) + ")",
                   cv::Point(static_cast<int>(blueMidpoint.x) - 40, static_cast<int>(blueMidpoint.y) - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        
        // Calculate angle
        blueGoalAngle = findGoalAngle(blueMidpoint);
        cv::putText(output, "Angle: " + std::to_string(static_cast<int>(blueGoalAngle * 180.0 / M_PI)) + " deg",
                   cv::Point(static_cast<int>(blueMidpoint.x) - 40, static_cast<int>(blueMidpoint.y) + 40),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
    }
    
    // Step 5: Process yellow goalpost (similar to blue processing)
    yellowDetected = false;
    cv::Mat yellowQuad;
    
    std::vector<std::vector<cv::Point>> filteredYellow = filterOutRods(yellowContours);
    if (filteredYellow.size() > 1) {
        std::vector<cv::Point> combinedYellow = combineGoalpostParts(filteredYellow, MIN_CONTOUR_AREA / 2);
        if (!combinedYellow.empty()) {
            yellowQuad = detectQuadrilateral(combinedYellow, EPSILON_FACTOR);
            if (!yellowQuad.empty()) {
                yellowDetected = true;
            }
        }
    }
    
    if (!yellowDetected && !filteredYellow.empty()) {
        std::vector<cv::Point> reliableYellow = getReliableGoalpostContour(filteredYellow, yellowMask);
        if (!reliableYellow.empty()) {
            yellowQuad = detectQuadrilateral(reliableYellow, EPSILON_FACTOR);
            if (!yellowQuad.empty()) {
                yellowDetected = true;
            }
        }
    }
    
    if (!yellowDetected && !yellowContours.empty()) {
        // Find largest contour
        int largestIdx = 0;
        double largestArea = 0;
        
        for (int i = 0; i < yellowContours.size(); i++) {
            double area = cv::contourArea(yellowContours[i]);
            if (area > largestArea) {
                largestArea = area;
                largestIdx = i;
            }
        }
        
        if (largestArea > MIN_CONTOUR_AREA) {
            yellowQuad = detectQuadrilateral(yellowContours[largestIdx], EPSILON_FACTOR);
            if (!yellowQuad.empty()) {
                yellowDetected = true;
            }
        }
    }
    
    // Process yellow quadrilateral if detected
    if (yellowDetected) {
        // Draw quadrilateral
        for (int i = 0; i < 4; i++) {
            cv::Point pt1(static_cast<int>(yellowQuad.at<float>(i, 0)), 
                         static_cast<int>(yellowQuad.at<float>(i, 1)));
            cv::Point pt2(static_cast<int>(yellowQuad.at<float>((i + 1) % 4, 0)), 
                         static_cast<int>(yellowQuad.at<float>((i + 1) % 4, 1)));
            cv::line(output, pt1, pt2, cv::Scalar(0, 255, 255), 2);
        }
        
        // Find closest points to center
        std::vector<cv::Point> closestPoints = findClosestPointsToCenter(yellowQuad);
        cv::Point point1 = closestPoints[0];
        cv::Point point2 = closestPoints[1];
        
        cv::Point2f yellowBottomMidpoint = findLineMidpoint({point1, point2});
        yellowGoalDistance = findDistanceToGoal(yellowBottomMidpoint);
        std::cout << "Distance to Yellow Goal: " << yellowGoalDistance << std::endl;
        
        cv::circle(output, point1, 7, cv::Scalar(255, 0, 0), -1);
        cv::circle(output, point2, 7, cv::Scalar(255, 0, 0), -1);
        cv::line(output, point1, point2, cv::Scalar(255, 0, 0), 2);
        
        int yellowMidpointX = static_cast<int>(yellowBottomMidpoint.x);
        int yellowMidpointY = static_cast<int>(yellowBottomMidpoint.y);
        cv::circle(output, cv::Point(yellowMidpointX, yellowMidpointY), 7, cv::Scalar(0, 0, 255), -1);
        
        std::cout << "Yellow closest points to center: " << point1 << ", " << point2 << std::endl;
        
        // Find bottom edge and nearest field point
        int bottomEdgeY = findTrueBottomEdgeQuadrilateral(frame, yellowQuad, YELLOW_LOWER, YELLOW_UPPER);
        cv::Point nearestPoint = findNearestFieldPoint(yellowQuad, bottomEdgeY, frame.rows);
        cv::Point2f yellowMidpoint = findQuadMidpoint(yellowQuad);
        
        cv::circle(output, nearestPoint, 5, cv::Scalar(0, 255, 255), -1);
        cv::circle(output, cv::Point(static_cast<int>(yellowMidpoint.x), static_cast<int>(yellowMidpoint.y)), 
                  5, cv::Scalar(255, 255, 255), -1);
                  
        cv::line(output, cv::Point(nearestPoint.x - 10, nearestPoint.y), 
                cv::Point(nearestPoint.x + 10, nearestPoint.y), cv::Scalar(0, 255, 255), 2);
                
        // Draw text info
        cv::putText(output, "(" + std::to_string(nearestPoint.x) + "," + std::to_string(nearestPoint.y) + ")",
                   cv::Point(nearestPoint.x - 40, nearestPoint.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
                   
        cv::putText(output, "(" + std::to_string(static_cast<int>(yellowMidpoint.x)) + "," + 
                   std::to_string(static_cast<int>(yellowMidpoint.y)) + ")",
                   cv::Point(static_cast<int>(yellowMidpoint.x) - 40, static_cast<int>(yellowMidpoint.y) - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        
        // Calculate angle
        yellowGoalAngle = findGoalAngle(yellowMidpoint);
        cv::putText(output, "Angle: " + std::to_string(static_cast<int>(yellowGoalAngle * 180.0 / M_PI)) + " deg",
                   cv::Point(static_cast<int>(yellowMidpoint.x) - 40, static_cast<int>(yellowMidpoint.y) + 40),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
    }
    
    // Debug visualization
    if (DEBUG) {
        cv::Mat blueDisplay, yellowDisplay;
        cv::cvtColor(blueMask, blueDisplay, cv::COLOR_GRAY2BGR);
        cv::cvtColor(yellowMask, yellowDisplay, cv::COLOR_GRAY2BGR);
        
        cv::putText(blueDisplay, "Blue Mask", cv::Point(10, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
                   
        cv::putText(yellowDisplay, "Yellow Mask", cv::Point(10, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        
        int h = frame.rows, w = frame.cols;
        cv::Mat blueResized, yellowResized;
        cv::resize(blueDisplay, blueResized, cv::Size(w / 2, h / 2));
        cv::resize(yellowDisplay, yellowResized, cv::Size(w / 2, h / 2));
        
        cv::Mat debugTop;
        cv::hconcat(blueResized, yellowResized, debugTop);
        
        cv::Mat outputResized;
        cv::resize(output, outputResized, cv::Size(w, h / 2));
        
        cv::Mat debugView;
        cv::vconcat(debugTop, outputResized, debugView);
        
        cv::imshow("Debug View", debugView);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "Time taken for frame: " << duration.count() << "ms" << std::endl;
    
    return output;
}

void GoalpostDetector::run() {
    if (!initialize()) {
        std::cerr << "Failed to initialize detector" << std::endl;
        return;
    }
    
    while (cap.isOpened()) {
        cv::Mat frame;
        
        // Read frame
        if (!cap.read(frame)) {
            break;
        }
        
        // Process frame
        bool blueDetected = false;
        bool yellowDetected = false;
        double blueGoalAngle = 0.0;
        double yellowGoalAngle = 0.0;
        double blueGoalDistance = 0.0;
        double yellowGoalDistance = 0.0;
        
        cv::Mat output = processFrame(frame, blueDetected, yellowDetected, 
                                    blueGoalAngle, yellowGoalAngle,
                                    blueGoalDistance, yellowGoalDistance);
        
        // Display output
        cv::imshow("Goal Detection", output);
        
        // Check for user input
        int key = cv::waitKey(25);
        if (key == 'q') {
            break;
        }
    }
    
    cap.release();
    cv::destroyAllWindows();
}