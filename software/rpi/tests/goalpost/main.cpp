
#include <filesystem>
#include <iostream>
#include <string>
#include "goalpost_detection.hpp"

int main() {
    // Open video
    std::string videoPath = "RBC-soccer-2025/software/rpi-direct/libs/camera/scripts/vid.mp4";
    std::cout << "File exists? " << std::filesystem::exists(videoPath) << std::endl;
    
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file" << std::endl;
        return -1;
    }
    
    // Main processing loop
    while (cap.isOpened()) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            break;
        }
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        cv::Mat hsv, output = frame.clone();
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        
        // Create and clean color masks
        cv::Mat blueMask, yellowMask;
        cv::inRange(hsv, goalpost::BLUE_LOWER, goalpost::BLUE_UPPER, blueMask);
        cv::inRange(hsv, goalpost::YELLOW_LOWER, goalpost::YELLOW_UPPER, yellowMask);
        
        // Apply morphology
        cv::Mat kernelSmall = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        
        cv::morphologyEx(blueMask, blueMask, cv::MORPH_CLOSE, kernelSmall);
        cv::morphologyEx(yellowMask, yellowMask, cv::MORPH_CLOSE, kernelSmall);
        
        // Find contours
        std::vector<std::vector<cv::Point>> blueContours, yellowContours;
        cv::findContours(blueMask, blueContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::findContours(yellowMask, yellowContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        // Process blue goalpost
        bool blueDetected = false;
        std::vector<cv::Point2f> blueQuad;
        
        // Try approach 1: Filter and combine
        std::vector<std::vector<cv::Point>> filteredBlue = goalpost::filterOutRods(blueContours);
        if (filteredBlue.size() > 1) {
            std::vector<cv::Point> combinedBlue = goalpost::combineGoalpostParts(filteredBlue, goalpost::MIN_CONTOUR_AREA / 2);
            if (!combinedBlue.empty()) {
                blueQuad = goalpost::detectQuadrilateral(combinedBlue, goalpost::EPSILON_FACTOR);
                if (!blueQuad.empty()) {
                    blueDetected = true;
                }
            }
        }
        
        // Try approach 2: Use reliable goalpost contour
        if (!blueDetected && !filteredBlue.empty()) {
            std::vector<cv::Point> reliableBlue = goalpost::getReliableGoalpostContour(filteredBlue, blueMask);
            if (!reliableBlue.empty()) {
                blueQuad = goalpost::detectQuadrilateral(reliableBlue, goalpost::EPSILON_FACTOR);
                if (!blueQuad.empty()) {
                    blueDetected = true;
                }
            }
        }
        
        // Try approach 3: Use largest contour directly
        if (!blueDetected && !blueContours.empty()) {
            // Find largest contour
            int largestIdx = 0;
            double largestArea = 0;
            for (size_t i = 0; i < blueContours.size(); i++) {
                double area = cv::contourArea(blueContours[i]);
                if (area > largestArea) {
                    largestArea = area;
                    largestIdx = i;
                }
            }
            
            if (largestArea > goalpost::MIN_CONTOUR_AREA) {
                blueQuad = goalpost::detectQuadrilateral(blueContours[largestIdx], goalpost::EPSILON_FACTOR);
                if (!blueQuad.empty()) {
                    blueDetected = true;
                }
            }
        }
        
        // Draw blue goalpost if detected
        if (blueDetected) {
            // Draw quadrilateral
            for (size_t i = 0; i < 4; i++) {
                cv::Point pt1(static_cast<int>(blueQuad[i].x), static_cast<int>(blueQuad[i].y));
                cv::Point pt2(static_cast<int>(blueQuad[(i + 1) % 4].x), static_cast<int>(blueQuad[(i + 1) % 4].y));
                cv::line(output, pt1, pt2, cv::Scalar(255, 0, 0), 2);
            }
            
            // Find true bottom edge
            int bottomEdgeY = goalpost::findTrueBottomEdgeQuadrilateral(frame, blueQuad, goalpost::BLUE_LOWER, goalpost::BLUE_UPPER);
            
            // Find nearest field point
            cv::Point2f nearestPoint = goalpost::findNearestFieldPoint(blueQuad, bottomEdgeY, frame.rows);
            int nearestX = static_cast<int>(nearestPoint.x);
            int nearestY = static_cast<int>(nearestPoint.y);
            
            // Calculate midpoint
            cv::Point2f blueMidpoint = goalpost::findQuadMidpoint(blueQuad);
            std::cout << "Blue midpoint: " << blueMidpoint << std::endl;
            
            // Draw points and annotations
            cv::circle(output, cv::Point(nearestX, nearestY), 5, cv::Scalar(255, 255, 0), -1);
            cv::circle(output, cv::Point(static_cast<int>(blueMidpoint.x), static_cast<int>(blueMidpoint.y)), 
                      5, cv::Scalar(255, 255, 255), -1);
            
            cv::line(output, cv::Point(nearestX - 10, nearestY), cv::Point(nearestX + 10, nearestY), 
                    cv::Scalar(255, 255, 0), 2);
            
            cv::putText(output, "(" + std::to_string(nearestX) + "," + std::to_string(nearestY) + ")",
                       cv::Point(nearestX - 40, nearestY - 10), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
                       
            cv::putText(output, 
                       "(" + std::to_string(static_cast<int>(blueMidpoint.x)) + "," + 
                       std::to_string(static_cast<int>(blueMidpoint.y)) + ")",
                       cv::Point(static_cast<int>(blueMidpoint.x) - 40, static_cast<int>(blueMidpoint.y) - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
        
        // Process yellow goalpost (similar to blue)
        bool yellowDetected = false;
        std::vector<cv::Point2f> yellowQuad;
        
        // Try approach 1: Filter and combine
        std::vector<std::vector<cv::Point>> filteredYellow = goalpost::filterOutRods(yellowContours);
        if (filteredYellow.size() > 1) {
            std::vector<cv::Point> combinedYellow = goalpost::combineGoalpostParts(filteredYellow, goalpost::MIN_CONTOUR_AREA / 2);
            if (!combinedYellow.empty()) {
                yellowQuad = goalpost::detectQuadrilateral(combinedYellow, goalpost::EPSILON_FACTOR);
                if (!yellowQuad.empty()) {
                    yellowDetected = true;
                }
            }
        }
        
        // Try approach 2: Use reliable goalpost contour
        if (!yellowDetected && !filteredYellow.empty()) {
            std::vector<cv::Point> reliableYellow = goalpost::getReliableGoalpostContour(filteredYellow, yellowMask);
            if (!reliableYellow.empty()) {
                yellowQuad = goalpost::detectQuadrilateral(reliableYellow, goalpost::EPSILON_FACTOR);
                if (!yellowQuad.empty()) {
                    yellowDetected = true;
                }
            }
        }
        
        // Try approach 3: Use largest contour directly
        if (!yellowDetected && !yellowContours.empty()) {
            // Find largest contour
            int largestIdx = 0;
            double largestArea = 0;
            for (size_t i = 0; i < yellowContours.size(); i++) {
                double area = cv::contourArea(yellowContours[i]);
                if (area > largestArea) {
                    largestArea = area;
                    largestIdx = i;
                }
            }
            
            if (largestArea > goalpost::MIN_CONTOUR_AREA) {
                yellowQuad = goalpost::detectQuadrilateral(yellowContours[largestIdx], goalpost::EPSILON_FACTOR);
                if (!yellowQuad.empty()) {
                    yellowDetected = true;
                }
            }
        }
        
        // Draw yellow goalpost if detected
        if (yellowDetected) {
            // Draw quadrilateral
            for (size_t i = 0; i < 4; i++) {
                cv::Point pt1(static_cast<int>(yellowQuad[i].x), static_cast<int>(yellowQuad[i].y));
                cv::Point pt2(static_cast<int>(yellowQuad[(i + 1) % 4].x), static_cast<int>(yellowQuad[(i + 1) % 4].y));
                cv::line(output, pt1, pt2, cv::Scalar(0, 255, 255), 2);
            }
            
            // Find true bottom edge
            int bottomEdgeY = goalpost::findTrueBottomEdgeQuadrilateral(frame, yellowQuad, goalpost::YELLOW_LOWER, goalpost::YELLOW_UPPER);
            
            // Find nearest field point
            cv::Point2f nearestPoint = goalpost::findNearestFieldPoint(yellowQuad, bottomEdgeY, frame.rows);
            int nearestX = static_cast<int>(nearestPoint.x);
            int nearestY = static_cast<int>(nearestPoint.y);
            
            // Calculate midpoint
            cv::Point2f yellowMidpoint = goalpost::findQuadMidpoint(yellowQuad);
            
            // Draw points and annotations
            cv::circle(output, cv::Point(nearestX, nearestY), 5, cv::Scalar(0, 255, 255), -1);
            cv::circle(output, cv::Point(static_cast<int>(yellowMidpoint.x), static_cast<int>(yellowMidpoint.y)),
                      5, cv::Scalar(255, 255, 255), -1);
            
            cv::line(output, cv::Point(nearestX - 10, nearestY), cv::Point(nearestX + 10, nearestY),
                    cv::Scalar(0, 255, 255), 2);
            
            cv::putText(output, "(" + std::to_string(nearestX) + "," + std::to_string(nearestY) + ")",
                       cv::Point(nearestX - 40, nearestY - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
                       
            cv::putText(output,
                       "(" + std::to_string(static_cast<int>(yellowMidpoint.x)) + "," +
                       std::to_string(static_cast<int>(yellowMidpoint.y)) + ")",
                       cv::Point(static_cast<int>(yellowMidpoint.x) - 40, static_cast<int>(yellowMidpoint.y) - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
        
        // Debug visualization
        if (goalpost::DEBUG) {
            // Create colored versions of masks
            cv::Mat blueDisplay, yellowDisplay;
            cv::cvtColor(blueMask, blueDisplay, cv::COLOR_GRAY2BGR);
            cv::cvtColor(yellowMask, yellowDisplay, cv::COLOR_GRAY2BGR);
            
            // Add labels
            cv::putText(blueDisplay, "Blue Mask", cv::Point(10, 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
            cv::putText(yellowDisplay, "Yellow Mask", cv::Point(10, 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
            
            // Resize masks to half size
            cv::resize(blueDisplay, blueDisplay, cv::Size(frame.cols / 2, frame.rows / 2));
            cv::resize(yellowDisplay, yellowDisplay, cv::Size(frame.cols / 2, frame.rows / 2));
            
            // Create composite debug view
            cv::Mat debugTop, debugView, outputResized;
            cv::hconcat(blueDisplay, yellowDisplay, debugTop);
            cv::resize(output, outputResized, cv::Size(frame.cols, frame.rows / 2));
            cv::vconcat(debugTop, outputResized, debugView);
            
            cv::imshow("Debug View", debugView);
        }
        
        // Show main output
        cv::imshow("Goal Detection", output);
        
        // Calculate processing time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        std::cout << "Time taken for frame: " << duration << " ms" << std::endl;
        
        // Exit if 'q' is pressed
        if (cv::waitKey(25) == 'q') {
            break;
        }
    }
    
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
}