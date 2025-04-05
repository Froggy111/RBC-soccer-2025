#include "goalpost.hpp"

GoalpostDetector::GoalpostDetector(bool debug) : m_debug(debug) {
    // Initialize HSV color thresholds
    BLUE_LOWER = cv::Scalar(100, 50, 50);
    BLUE_UPPER = cv::Scalar(150, 255, 255);

    YELLOW_LOWER = cv::Scalar(20, 137, 110);
    YELLOW_UPPER = cv::Scalar(30, 255, 255);

    FIELD_LOWER = cv::Scalar(35, 50, 50);
    FIELD_UPPER = cv::Scalar(85, 255, 255);

    // Initialize parameters
    MIN_CONTOUR_AREA   = 400;
    MIN_GOAL_HEIGHT    = 10;
    EDGE_SEARCH_HEIGHT = 69;
    EPSILON_FACTOR     = 0.05;

    // Default center point
    m_centerPoint = cv::Point(640 / 2 - 5, 480 / 2 + 30);
}

void GoalpostDetector::setCenterPoint(const cv::Point &center) {
    m_centerPoint = center;
}

std::pair<GoalpostInfo, GoalpostInfo>
GoalpostDetector::detectGoalposts(const cv::Mat &frame, cv::Mat *outputFrame) {

    // Initialize result structures
    GoalpostInfo blueGoalInfo   = {false};
    GoalpostInfo yellowGoalInfo = {false};

    // Create a copy for output visualization if requested
    cv::Mat output;
    if (outputFrame) {
        output = frame.clone();
        cv::circle(output, m_centerPoint, 5, cv::Scalar(255, 255, 255), -1);
    }

    // Convert to HSV color space
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // Create color masks
    cv::Mat blueMask, yellowMask;
    cv::inRange(hsv, BLUE_LOWER, BLUE_UPPER, blueMask);
    cv::inRange(hsv, YELLOW_LOWER, YELLOW_UPPER, yellowMask);

    // Apply morphology
    cv::Mat kernelSmall =
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(blueMask, blueMask, cv::MORPH_CLOSE, kernelSmall);
    cv::morphologyEx(yellowMask, yellowMask, cv::MORPH_CLOSE, kernelSmall);

    // Find contours
    std::vector<std::vector<cv::Point>> blueContours, yellowContours;
    cv::findContours(blueMask, blueContours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(yellowMask, yellowContours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    // Process blue goalpost
    std::vector<cv::Point> filteredBlue = filterOutRods(blueContours);
    std::vector<cv::Point> blueQuad;

    if (!filteredBlue.empty()) {
        // Try to combine parts first
        if (filteredBlue.size() > 1) {
            std::vector<cv::Point> combinedBlue =
                combineGoalpostParts(blueContours);
            if (!combinedBlue.empty()) {
                blueQuad              = detectQuadrilateral(combinedBlue);
                blueGoalInfo.detected = !blueQuad.empty();
            }
        }

        // Try getting a reliable contour if combining didn't work
        if (!blueGoalInfo.detected) {
            // Wrap single contour in a vector of contours
            std::vector<std::vector<cv::Point>> contourVec = {filteredBlue};
            std::vector<cv::Point> reliableBlue =
                getReliableGoalpostContour(contourVec);
            if (!reliableBlue.empty()) {
                blueQuad              = detectQuadrilateral(reliableBlue);
                blueGoalInfo.detected = !blueQuad.empty();
            }
        }
    }

    // Process blue goalpost data if detected
    if (blueGoalInfo.detected) {
        blueGoalInfo.quad = blueQuad;

        // Draw quadrilateral if output is requested
        if (outputFrame) {
            for (size_t i = 0; i < blueQuad.size(); i++) {
                cv::line(output, blueQuad[i],
                         blueQuad[(i + 1) % blueQuad.size()],
                         cv::Scalar(255, 0, 0), 2);
            }
        }

        // Find closest points to center
        auto [closestPoint1, closestPoint2] =
            findClosestPointsToCenter(blueQuad);
        cv::Point bottomMidpoint = (closestPoint1 + closestPoint2) / 2;

        // Calculate distance and angle
        blueGoalInfo.distance = findDistanceToGoal(bottomMidpoint);
        blueGoalInfo.midpoint = findQuadMidpoint(blueQuad);
        blueGoalInfo.angle    = findGoalAngle(blueGoalInfo.midpoint);

        // Visualize if output requested
        if (outputFrame) {
            cv::circle(output, closestPoint1, 7, cv::Scalar(255, 0, 0), -1);
            cv::circle(output, closestPoint2, 7, cv::Scalar(255, 0, 0), -1);
            cv::line(output, closestPoint1, closestPoint2,
                     cv::Scalar(255, 0, 0), 2);
            cv::circle(output, bottomMidpoint, 7, cv::Scalar(255, 255, 0), -1);

            // Draw midpoint
            cv::Point midpointInt(blueGoalInfo.midpoint.x,
                                  blueGoalInfo.midpoint.y);
            cv::circle(output, midpointInt, 5, cv::Scalar(255, 255, 255), -1);

            // Display angle
            std::string angleText =
                "Angle: " + std::to_string(blueGoalInfo.angle * 180 / M_PI) +
                " deg";
            cv::putText(output, angleText,
                        cv::Point(midpointInt.x - 40, midpointInt.y + 40),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255),
                        2);
        }
    }

    // Process yellow goalpost
    std::vector<cv::Point> filteredYellow = filterOutRods(yellowContours);
    std::vector<cv::Point> yellowQuad;

    if (!filteredYellow.empty()) {
        // Try to combine parts first
        if (filteredYellow.size() > 1) {
            std::vector<cv::Point> combinedYellow =
                combineGoalpostParts(yellowContours);
            if (!combinedYellow.empty()) {
                yellowQuad              = detectQuadrilateral(combinedYellow);
                yellowGoalInfo.detected = !yellowQuad.empty();
            }
        }

        // Try getting a reliable contour if combining didn't work
        if (!yellowGoalInfo.detected) {
            std::vector<std::vector<cv::Point>> contourVec = {filteredYellow};
            std::vector<cv::Point> reliableYellow =
                getReliableGoalpostContour(contourVec);
            if (!reliableYellow.empty()) {
                yellowQuad              = detectQuadrilateral(reliableYellow);
                yellowGoalInfo.detected = !yellowQuad.empty();
            }
        }
    }

    // Process yellow goalpost data if detected
    if (yellowGoalInfo.detected) {
        yellowGoalInfo.quad = yellowQuad;

        // Draw quadrilateral if output is requested
        if (outputFrame) {
            for (size_t i = 0; i < yellowQuad.size(); i++) {
                cv::line(output, yellowQuad[i],
                         yellowQuad[(i + 1) % yellowQuad.size()],
                         cv::Scalar(0, 255, 255), 2);
            }
        }

        // Find closest points to center
        auto [closestPoint1, closestPoint2] =
            findClosestPointsToCenter(yellowQuad);
        cv::Point bottomMidpoint = (closestPoint1 + closestPoint2) / 2;

        // Calculate distance and angle
        yellowGoalInfo.distance = findDistanceToGoal(bottomMidpoint);
        yellowGoalInfo.midpoint = findQuadMidpoint(yellowQuad);
        yellowGoalInfo.angle    = findGoalAngle(yellowGoalInfo.midpoint);

        // Visualize if output requested
        if (outputFrame) {
            cv::circle(output, closestPoint1, 7, cv::Scalar(255, 0, 0), -1);
            cv::circle(output, closestPoint2, 7, cv::Scalar(255, 0, 0), -1);
            cv::line(output, closestPoint1, closestPoint2,
                     cv::Scalar(255, 0, 0), 2);
            cv::circle(output, bottomMidpoint, 7, cv::Scalar(0, 0, 255), -1);

            // Draw midpoint
            cv::Point midpointInt(yellowGoalInfo.midpoint.x,
                                  yellowGoalInfo.midpoint.y);
            cv::circle(output, midpointInt, 5, cv::Scalar(255, 255, 255), -1);

            // Display angle
            std::string angleText =
                "Angle: " + std::to_string(yellowGoalInfo.angle * 180 / M_PI) +
                " deg";
            cv::putText(output, angleText,
                        cv::Point(midpointInt.x - 40, midpointInt.y + 40),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255),
                        2);
        }
    }

    // Set output image if requested
    if (outputFrame) {
        *outputFrame = output;
    }

    // Return the result
    return {blueGoalInfo, yellowGoalInfo};
}

std::vector<cv::Point>
GoalpostDetector::detectQuadrilateral(const std::vector<cv::Point> &contour) {
    std::vector<cv::Point> quad;

    double perimeter = cv::arcLength(contour, true);
    double epsilon   = EPSILON_FACTOR * perimeter;
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, epsilon, true);

    if (m_debug) {
        std::cout << "Contour has " << approx.size() << " points" << std::endl;
    }

    // Check if it's a valid shape
    if (approx.size() < 4 || approx.size() > 6) {
        if (m_debug) {
            std::cout << "Skipping shape with " << approx.size() << " points"
                      << std::endl;
        }
        return quad;
    }

    // If more than 4 points, take the 4 furthest from the center
    if (approx.size() > 4) {
        cv::Point2f center(0, 0);
        for (const auto &pt : approx) {
            center +=
                cv::Point2f(pt.x, pt.y); // Convert cv::Point to cv::Point2f
        }
        center = center * (1.0f / approx.size());

        // Calculate distances
        std::vector<std::pair<int, float>> distances;
        for (size_t i = 0; i < approx.size(); i++) {
            // Convert to Point2f before subtraction
            float dist =
                cv::norm(cv::Point2f(approx[i].x, approx[i].y) - center);
            distances.push_back({i, dist});
        }

        // Sort by distance (descending)
        std::sort(
            distances.begin(), distances.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

        // Take top 4 points
        for (int i = 0; i < 4; i++) {
            quad.push_back(approx[distances[i].first]);
        }
    } else {
        quad = approx;
    }

    // Check height/width ratio
    cv::Rect boundRect = cv::boundingRect(quad);
    if (boundRect.height < MIN_GOAL_HEIGHT || boundRect.width < 5 ||
        static_cast<float>(boundRect.height) / boundRect.width > 8.0) {
        if (m_debug) {
            std::cout << "Skipping shape with h/w ratio "
                      << static_cast<float>(boundRect.height) / boundRect.width
                      << std::endl;
        }
        return std::vector<cv::Point>();
    }

    // Order points consistently
    if (quad.size() == 4) {
        quad = orderQuadrilateralPoints(quad);
    }

    return quad;
}

std::vector<cv::Point>
GoalpostDetector::orderQuadrilateralPoints(std::vector<cv::Point> pts) {
    // Sort points by x-coordinate
    std::vector<cv::Point> result(4);
    std::sort(pts.begin(), pts.end(),
              [](const cv::Point &a, const cv::Point &b) { return a.x < b.x; });

    std::vector<cv::Point> leftPoints  = {pts[0], pts[1]};
    std::vector<cv::Point> rightPoints = {pts[2], pts[3]};

    // Sort left points by y-coordinate
    std::sort(leftPoints.begin(), leftPoints.end(),
              [](const cv::Point &a, const cv::Point &b) { return a.y < b.y; });

    // Sort right points by y-coordinate
    std::sort(rightPoints.begin(), rightPoints.end(),
              [](const cv::Point &a, const cv::Point &b) { return a.y < b.y; });

    // Return ordered points: top-left, top-right, bottom-right, bottom-left
    return {leftPoints[0], rightPoints[0], rightPoints[1], leftPoints[1]};
}

int GoalpostDetector::findTrueBottomEdge(const cv::Mat &frame,
                                         const std::vector<cv::Point> &quad,
                                         const cv::Scalar &goalLower,
                                         const cv::Scalar &goalUpper) {
    // Get bounding rectangle
    cv::Rect boundRect = cv::boundingRect(quad);
    int x              = boundRect.x;
    int y              = boundRect.y;
    int w              = boundRect.width;
    int h              = boundRect.height;

    // Create ROI
    int roiHeight = std::min(h + EDGE_SEARCH_HEIGHT, frame.rows - y);
    cv::Mat roi   = frame(cv::Rect(x, y, w, roiHeight));

    // Create mask from quadrilateral
    cv::Mat mask = cv::Mat::zeros(roiHeight, w, CV_8UC1);
    std::vector<cv::Point> shiftedQuad;
    for (const auto &pt : quad) {
        shiftedQuad.push_back(cv::Point(pt.x - x, pt.y - y));
    }
    std::vector<std::vector<cv::Point>> contours = {shiftedQuad};
    cv::fillPoly(mask, contours, 255);

    // Convert to HSV and create masks
    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    cv::Mat goalMask, fieldMask;
    cv::inRange(hsv, goalLower, goalUpper, goalMask);
    cv::inRange(hsv, FIELD_LOWER, FIELD_UPPER, fieldMask);
    cv::bitwise_and(goalMask, mask, goalMask);
    cv::bitwise_and(fieldMask, mask, fieldMask);

    // Find vertical projection
    std::vector<int> verticalProj(roiHeight, 0);
    for (int r = 0; r < roiHeight; r++) {
        verticalProj[r] = cv::sum(goalMask.row(r))[0];
    }

    // Find threshold and goal rows
    double maxVal = 0;
    for (int val : verticalProj)
        maxVal = std::max(maxVal, static_cast<double>(val));
    double threshold = 0.2 * maxVal;

    int bottomRow = h;
    for (int r = 0; r < roiHeight; r++) {
        if (verticalProj[r] > threshold) {
            bottomRow = r;
        }
    }

    // Look for field transition
    for (int r = bottomRow;
         r < std::min(bottomRow + EDGE_SEARCH_HEIGHT, roiHeight); r++) {
        if (r >= fieldMask.rows)
            break;
        if (cv::sum(fieldMask.row(r))[0] > 0.05 * w * 255) {
            bottomRow = r;
            break;
        }
    }

    return y + bottomRow;
}

std::vector<cv::Point> GoalpostDetector::filterOutRods(
    const std::vector<std::vector<cv::Point>> &contours) {

    std::vector<std::vector<cv::Point>> filteredContours;
    for (const auto &contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float width          = rect.size.width;
        float height         = rect.size.height;

        // Ensure width is smaller than height
        if (width > height) {
            std::swap(width, height);
        }

        // Check aspect ratio
        float aspectRatio = height / std::max(width, 1.0f);
        if (aspectRatio <= 8.0) {
            filteredContours.push_back(contour);
        }
    }

    // Return the largest filtered contour or an empty vector
    if (filteredContours.empty()) {
        return std::vector<cv::Point>();
    }

    auto largestContour = *std::max_element(
        filteredContours.begin(), filteredContours.end(),
        [](const std::vector<cv::Point> &a, const std::vector<cv::Point> &b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });

    return largestContour;
}

std::vector<cv::Point> GoalpostDetector::combineGoalpostParts(
    const std::vector<std::vector<cv::Point>> &contours) {

    std::vector<std::vector<cv::Point>> significantContours;
    for (const auto &contour : contours) {
        if (cv::contourArea(contour) > (float)MIN_CONTOUR_AREA / 4) {
            significantContours.push_back(contour);
        }
    }

    if (significantContours.size() > 1) {
        // Combine all points into one vector
        std::vector<cv::Point> allPoints;
        for (const auto &contour : significantContours) {
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

std::vector<cv::Point> GoalpostDetector::getReliableGoalpostContour(
    const std::vector<std::vector<cv::Point>> &contours) {

    if (contours.empty()) {
        return std::vector<cv::Point>();
    }

    // Sort contours by area
    std::vector<std::vector<cv::Point>> sortedContours = contours;
    std::sort(
        sortedContours.begin(), sortedContours.end(),
        [](const std::vector<cv::Point> &a, const std::vector<cv::Point> &b) {
            return cv::contourArea(a) > cv::contourArea(b);
        });

    // Check if largest contour is big enough
    std::vector<cv::Point> largest = sortedContours[0];
    if (cv::contourArea(largest) < MIN_CONTOUR_AREA) {
        return std::vector<cv::Point>();
    }

    // Create convex hull
    std::vector<cv::Point> hull;
    cv::convexHull(largest, hull);

    // Check aspect ratio
    cv::Rect boundRect = cv::boundingRect(hull);
    float aspectRatio =
        static_cast<float>(boundRect.height) / std::max(boundRect.width, 1);

    if (aspectRatio > 1.5) {
        return hull;
    }

    return largest;
}

std::pair<cv::Point, cv::Point> GoalpostDetector::findClosestPointsToCenter(
    const std::vector<cv::Point> &quad) {

    // Calculate distances to center
    std::vector<std::pair<int, float>> distances;
    for (size_t i = 0; i < quad.size(); i++) {
        float dist = cv::norm(quad[i] - m_centerPoint);
        distances.push_back({i, dist});
    }

    // Sort by distance
    std::sort(distances.begin(), distances.end(),
              [](const auto &a, const auto &b) { return a.second < b.second; });

    // Return the two closest points
    return {quad[distances[0].first], quad[distances[1].first]};
}

cv::Point
GoalpostDetector::findQuadMidpoint(const std::vector<cv::Point> &quad) {
    cv::Point sum(0, 0);
    for (const auto &pt : quad) {
        sum += pt;
    }
    return cv::Point(sum.x / quad.size(), sum.y / quad.size());
}

float GoalpostDetector::findGoalAngle(const cv::Point &goalMidpoint) {
    float deltaX = goalMidpoint.x - m_centerPoint.x;
    float deltaY = goalMidpoint.y - m_centerPoint.y;
    float angle  = atan2(deltaY, deltaX);
    return angle + M_PI / 2; // Adjust to make forward = 0
}

float GoalpostDetector::findDistanceToGoal(const cv::Point &goalMidpoint) {
    float deltaX = goalMidpoint.x - m_centerPoint.x;
    float deltaY = goalMidpoint.y - m_centerPoint.y;
    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

cv::Point
GoalpostDetector::findNearestFieldPoint(const std::vector<cv::Point> &quad,
                                        int bottomEdgeY, int frameHeight) {

    int botCenterX     = frameHeight / 2;
    int botCenterY     = frameHeight;
    int fieldThreshold = 20;
    std::vector<cv::Point> fieldPoints;

    // Find points near the field edge
    for (const auto &point : quad) {
        if (abs(point.y - bottomEdgeY) < fieldThreshold) {
            fieldPoints.push_back(point);
        }
    }

    if (fieldPoints.empty()) {
        fieldPoints = quad;
    }

    // Find closest point to robot center
    cv::Point closestPoint;
    float minDistance = std::numeric_limits<float>::max();

    for (const auto &point : fieldPoints) {
        float distance =
            sqrt(pow(point.x - botCenterX, 2) + pow(point.y - botCenterY, 2));
        if (distance < minDistance) {
            minDistance  = distance;
            closestPoint = point;
        }
    }

    return closestPoint;
}