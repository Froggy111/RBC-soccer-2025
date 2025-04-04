#include "include/goalpost_detection.hpp"

namespace goalpost {

// Find the true bottom edge of a rectangular region
int findTrueBottomEdge(const cv::Mat& frame, const cv::Rect& bbox, 
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

    // Calculate vertical projection
    std::vector<int> verticalProj(roiHeight, 0);
    for (int i = 0; i < roiHeight; i++) {
        verticalProj[i] = cv::sum(goalMask.row(i))[0];
    }

    // Find max value for threshold calculation
    int maxVal = 0;
    for (int val : verticalProj) {
        maxVal = std::max(maxVal, val);
    }
    
    int threshold = 0.4 * maxVal;
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

// Find the true bottom edge using a quadrilateral region
int findTrueBottomEdgeQuadrilateral(const cv::Mat& frame, const std::vector<cv::Point2f>& quadrilateral,
                                 const cv::Scalar& goalLower, const cv::Scalar& goalUpper) {
    cv::Rect bbox = cv::boundingRect(quadrilateral);
    int x = bbox.x;
    int y = bbox.y; 
    int w = bbox.width;
    int h = bbox.height;

    int roiHeight = std::min(h + EDGE_SEARCH_HEIGHT, frame.rows - y);
    cv::Mat roi = frame(cv::Rect(x, y, w, roiHeight));

    // Create mask from quadrilateral
    cv::Mat mask = cv::Mat::zeros(roiHeight, w, CV_8UC1);
    
    // Shift quadrilateral to ROI coordinates and convert to integer points
    std::vector<cv::Point> shiftedQuad;
    for (const auto& pt : quadrilateral) {
        cv::Point shiftedPt(
            std::max(0, std::min(w-1, int(pt.x - x))),
            std::max(0, std::min(roiHeight-1, int(pt.y - y)))
        );
        shiftedQuad.push_back(shiftedPt);
    }
    
    // Draw filled polygon
    std::vector<std::vector<cv::Point>> contours = {shiftedQuad};
    cv::fillPoly(mask, contours, 255);

    // Apply color detection
    cv::Mat hsv, goalMask, fieldMask;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    
    cv::Mat tempGoalMask, tempFieldMask;
    cv::inRange(hsv, goalLower, goalUpper, tempGoalMask);
    cv::inRange(hsv, FIELD_LOWER, FIELD_UPPER, tempFieldMask);
    
    // Apply the quadrilateral mask
    cv::bitwise_and(tempGoalMask, mask, goalMask);
    cv::bitwise_and(tempFieldMask, mask, fieldMask);

    // Calculate vertical projection
    std::vector<int> verticalProj(roiHeight, 0);
    for (int i = 0; i < roiHeight; i++) {
        verticalProj[i] = cv::sum(goalMask.row(i))[0];
    }

    // Find max for threshold
    int maxVal = 0;
    for (int val : verticalProj) {
        maxVal = std::max(maxVal, val);
    }
    
    int threshold = 0.2 * maxVal;
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

// Order points function
std::vector<cv::Point2f> orderPoints(const std::vector<cv::Point2f>& pts) {
    std::vector<cv::Point2f> rect(4);
    
    // Calculate sum of coordinates
    std::vector<float> sums(pts.size());
    for (size_t i = 0; i < pts.size(); i++) {
        sums[i] = pts[i].x + pts[i].y;
    }
    
    // Find indices for top-left (min sum) and bottom-right (max sum)
    auto minIt = std::min_element(sums.begin(), sums.end());
    auto maxIt = std::max_element(sums.begin(), sums.end());
    
    rect[0] = pts[std::distance(sums.begin(), minIt)]; // Top-left
    rect[2] = pts[std::distance(sums.begin(), maxIt)]; // Bottom-right
    
    // Calculate difference of coordinates
    std::vector<float> diffs(pts.size());
    for (size_t i = 0; i < pts.size(); i++) {
        diffs[i] = pts[i].y - pts[i].x;
    }
    
    // Find indices for top-right (min diff) and bottom-left (max diff)
    minIt = std::min_element(diffs.begin(), diffs.end());
    maxIt = std::max_element(diffs.begin(), diffs.end());
    
    rect[1] = pts[std::distance(diffs.begin(), minIt)]; // Top-right
    rect[3] = pts[std::distance(diffs.begin(), maxIt)]; // Bottom-left
    
    return rect;
}

// Detect quadrilateral from contour
std::vector<cv::Point2f> detectQuadrilateral(const std::vector<cv::Point>& contour, 
                                         double epsilonFactor,
                                         int minHeight, 
                                         int maxPoints) {
    // Approximate the contour
    double perimeter = cv::arcLength(contour, true);
    double epsilon = epsilonFactor * perimeter;
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, epsilon, true);
    
    // Debug output
    if (DEBUG) {
        std::cout << "Contour has " << approx.size() << " points" << std::endl;
    }
    
    // Check point count
    if (approx.size() < 4 || approx.size() > maxPoints) {
        if (DEBUG) {
            std::cout << "Skipping shape with " << approx.size() << " points" << std::endl;
        }
        return std::vector<cv::Point2f>();
    }
    
    std::vector<cv::Point2f> pts;
    
    // If we have more than 4 points, simplify to get 4 most significant
    if (approx.size() > 4) {
        // Calculate center of contour
        cv::Point2f center(0, 0);
        for (const auto& pt : approx) {
            center.x += pt.x;
            center.y += pt.y;
        }
        center.x /= approx.size();
        center.y /= approx.size();
        
        // Calculate distances from center
        std::vector<std::pair<float, int>> distances;
        for (size_t i = 0; i < approx.size(); i++) {
            float dx = approx[i].x - center.x;
            float dy = approx[i].y - center.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            distances.push_back(std::make_pair(dist, i));
        }
        
        // Sort distances in descending order
        std::sort(distances.begin(), distances.end(), 
                 [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
                     return a.first > b.first;
                 });
        
        // Take 4 points with largest distances
        for (int i = 0; i < 4; i++) {
            pts.push_back(cv::Point2f(approx[distances[i].second]));
        }
    } else {
        // Convert all points to Point2f
        for (const auto& pt : approx) {
            pts.push_back(cv::Point2f(pt));
        }
    }
    
    // Basic size check
    cv::Rect bbox = cv::boundingRect(pts);
    if (bbox.height < minHeight || bbox.width < 5 || 
        static_cast<float>(bbox.height) / std::max(1, bbox.width) > 8.0) {
        if (DEBUG) {
            std::cout << "Skipping shape with h/w ratio " 
                     << static_cast<float>(bbox.height) / std::max(1, bbox.width) << std::endl;
        }
        return std::vector<cv::Point2f>();
    }
    
    // Order points for consistency
    if (pts.size() == 4) {
        pts = orderQuadrilateralPoints(pts);
    }
    
    return pts;
}

// Order quadrilateral points
std::vector<cv::Point2f> orderQuadrilateralPoints(const std::vector<cv::Point2f>& pts) {
    // Sort by x coordinate
    std::vector<cv::Point2f> xSorted = pts;
    std::sort(xSorted.begin(), xSorted.end(), 
             [](const cv::Point2f& a, const cv::Point2f& b) {
                 return a.x < b.x;
             });
    
    // Get left points and right points
    std::vector<cv::Point2f> leftPoints = {xSorted[0], xSorted[1]};
    std::vector<cv::Point2f> rightPoints = {xSorted[2], xSorted[3]};
    
    // Sort left points by y
    std::sort(leftPoints.begin(), leftPoints.end(), 
             [](const cv::Point2f& a, const cv::Point2f& b) {
                 return a.y < b.y;
             });
    
    // Sort right points by y
    std::sort(rightPoints.begin(), rightPoints.end(), 
             [](const cv::Point2f& a, const cv::Point2f& b) {
                 return a.y < b.y;
             });
    
    // Return ordered points: top-left, top-right, bottom-right, bottom-left
    return {
        leftPoints[0],   // top-left
        rightPoints[0],  // top-right
        rightPoints[1],  // bottom-right
        leftPoints[1]    // bottom-left
    };
}

// Find nearest field point
cv::Point2f findNearestFieldPoint(const std::vector<cv::Point2f>& quadrilateral, 
                               int bottomEdgeY, int frameHeight) {
    // Get center bottom point (robot position)
    int botCenterX = frameHeight / 2;
    int botCenterY = frameHeight;
    
    // Filter points near field transition
    int fieldThreshold = 20;
    std::vector<cv::Point2f> fieldPoints;
    
    for (const auto& point : quadrilateral) {
        if (std::abs(point.y - bottomEdgeY) < fieldThreshold) {
            fieldPoints.push_back(point);
        }
    }
    
    // If no points near field line, use all points
    if (fieldPoints.empty()) {
        fieldPoints = quadrilateral;
    }
    
    // Find point closest to robot
    cv::Point2f closestPoint = fieldPoints[0];
    float minDistance = std::numeric_limits<float>::max();
    
    for (const auto& point : fieldPoints) {
        float dx = point.x - botCenterX;
        float dy = point.y - botCenterY;
        float distance = std::sqrt(dx*dx + dy*dy);
        
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = point;
        }
    }
    
    return closestPoint;
}

// Get reliable goalpost contour
std::vector<cv::Point> getReliableGoalpostContour(const std::vector<std::vector<cv::Point>>& contours, 
                                              const cv::Mat& colorMask) {
    if (contours.empty()) {
        return std::vector<cv::Point>();
    }
    
    // Sort contours by area (largest first)
    std::vector<std::pair<double, int>> areas;
    for (size_t i = 0; i < contours.size(); i++) {
        areas.push_back(std::make_pair(cv::contourArea(contours[i]), i));
    }
    
    std::sort(areas.begin(), areas.end(), 
             [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
                 return a.first > b.first;
             });
    
    const std::vector<cv::Point>& largest = contours[areas[0].second];
    
    // Check if it's large enough
    if (cv::contourArea(largest) < MIN_CONTOUR_AREA) {
        return std::vector<cv::Point>();
    }
    
    // Create convex hull
    std::vector<cv::Point> hull;
    cv::convexHull(largest, hull);
    
    // Calculate aspect ratio
    cv::Rect bbox = cv::boundingRect(hull);
    float aspectRatio = static_cast<float>(bbox.height) / std::max(1, bbox.width);
    
    // If tall and narrow, likely a goalpost
    if (aspectRatio > 1.5) {
        return hull;
    }
    
    return largest;  // Fall back to largest
}

// Filter out rod-like contours
std::vector<std::vector<cv::Point>> filterOutRods(const std::vector<std::vector<cv::Point>>& contours) {
    std::vector<std::vector<cv::Point>> filteredContours;
    
    for (const auto& contour : contours) {
        // Calculate minimum area rectangle
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float width = rect.size.width;
        float height = rect.size.height;
        
        // Ensure width < height for vertical objects
        if (width > height) {
            std::swap(width, height);
        }
        
        // Calculate aspect ratio
        float aspectRatio = height / std::max(1.0f, width);
        
        // If too tall and thin, likely a rod
        if (aspectRatio > 8.0) {
            continue;
        }
        
        filteredContours.push_back(contour);
    }
    
    return filteredContours;
}

// Combine goalpost parts
std::vector<cv::Point> combineGoalpostParts(const std::vector<std::vector<cv::Point>>& contours, int minArea) {
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
    } else if (significantContours.size() == 1) {
        return significantContours[0];
    }
    
    return std::vector<cv::Point>();
}

// Find quadrilateral midpoint
cv::Point2f findQuadMidpoint(const std::vector<cv::Point2f>& quadPoints) {
    float totalX = 0.0f;
    float totalY = 0.0f;
    
    for (const auto& pt : quadPoints) {
        totalX += pt.x;
        totalY += pt.y;
    }
    
    return cv::Point2f(totalX / quadPoints.size(), totalY / quadPoints.size());
}

}