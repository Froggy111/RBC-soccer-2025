import cv2
import numpy as np
import os
import datetime
import time

DEBUG = True

video_path = "RBC-soccer-2025/software/middle/goalpost_detection/vid.mp4"
print(
    "file exists? "
    + str(os.path.exists("RBC-soccer-2025/software/middle/goalpost_detection/vid.mp4"))
)
cap = cv2.VideoCapture(video_path)

BLUE_LOWER = np.array([70, 150, 50])
BLUE_UPPER = np.array([140, 255, 255])
YELLOW_LOWER = np.array([20, 100, 100])
YELLOW_UPPER = np.array([30, 255, 255])
FIELD_LOWER = np.array([35, 50, 50])
FIELD_UPPER = np.array([85, 255, 255])

MIN_CONTOUR_AREA = 500  # Decreased to detect smaller goalposts
MIN_GOAL_HEIGHT = 10
EDGE_SEARCH_HEIGHT = 69
EPSILON_FACTOR = 0.03  # Increased for more generous approximation
PARALLELOGRAM_TOLERANCE = 0.3  # Increased tolerance for parallelism


def find_true_bottom_edge(frame, bbox, goal_lower, goal_upper):
    """Finds the actual bottom edge by detecting field-goal transition"""
    x, y, w, h = bbox

    roi_height = min(h + EDGE_SEARCH_HEIGHT, frame.shape[0] - y)
    roi = frame[y : y + roi_height, x : x + w]

    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    goal_mask = cv2.inRange(hsv, goal_lower, goal_upper)
    field_mask = cv2.inRange(hsv, FIELD_LOWER, FIELD_UPPER)

    vertical_proj = np.sum(goal_mask, axis=1)

    threshold = 0.2 * np.max(vertical_proj) if len(vertical_proj) > 0 else 0
    goal_rows = np.where(vertical_proj > threshold)[0]

    if len(goal_rows) == 0:
        return y + h

    bottom_row = goal_rows[-1]
    for row in range(bottom_row, min(bottom_row + EDGE_SEARCH_HEIGHT, roi_height)):
        if row >= field_mask.shape[0]:
            break
        if np.sum(field_mask[row, :]) > 0.1 * w * 255:
            bottom_row = row
            break

    return y + bottom_row


def find_true_bottom_edge_parallelogram(frame, parallelogram, goal_lower, goal_upper):
    """Finds the actual bottom edge by detecting field-goal transition for a parallelogram"""
    # Get bounding box of the parallelogram
    x, y, w, h = cv2.boundingRect(parallelogram.reshape(-1, 1, 2))

    # Rest of your existing function remains the same
    roi_height = min(h + EDGE_SEARCH_HEIGHT, frame.shape[0] - y)
    roi = frame[y : y + roi_height, x : x + w]

    # Create a mask from the parallelogram for more precise ROI
    mask = np.zeros((roi_height, w), dtype=np.uint8)
    shifted_parallelogram = parallelogram - np.array([x, y])  # Shift to ROI coordinates
    cv2.fillPoly(mask, [shifted_parallelogram.reshape(-1, 1, 2)], 255)

    # Continue with your existing color detection but apply mask
    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    goal_mask = cv2.inRange(hsv, goal_lower, goal_upper) & mask
    field_mask = cv2.inRange(hsv, FIELD_LOWER, FIELD_UPPER) & mask

    # Continue with your existing bottom edge finding algorithm
    vertical_proj = np.sum(goal_mask, axis=1)
    # ... rest of your function


def detect_parallelogram(contour, epsilon_factor=0.05):  # Increased epsilon
    """
    Detect parallelogram from contour with more lenient criteria
    """
    # Approximate the contour
    perimeter = cv2.arcLength(contour, True)
    epsilon = epsilon_factor * perimeter
    approx = cv2.approxPolyDP(contour, epsilon, True)

    # Check if it's a quadrilateral (4 points)
    if len(approx) != 4 and len(approx) != 5:
        if DEBUG:
            #print(f"Skipping shape with {len(approx)} points")
            pass
        return None

    # Convert to simple points array - just return it with minimal checks
    pts = np.array([point[0] for point in approx])

    # Just make sure it's big enough
    x, y, w, h = cv2.boundingRect(approx)
    if h < MIN_GOAL_HEIGHT:
        if DEBUG:
            print(f"Height too small: {h}")
        return None

    return pts


while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    output = frame.copy()

    blue_mask = cv2.inRange(hsv, BLUE_LOWER, BLUE_UPPER)
    yellow_mask = cv2.inRange(hsv, YELLOW_LOWER, YELLOW_UPPER)

    # Add this before finding contours
    kernel = np.ones((5, 5), np.uint8)
    blue_mask = cv2.morphologyEx(blue_mask, cv2.MORPH_CLOSE, kernel)
    blue_mask = cv2.morphologyEx(blue_mask, cv2.MORPH_OPEN, kernel)
    yellow_mask = cv2.morphologyEx(yellow_mask, cv2.MORPH_CLOSE, kernel)
    yellow_mask = cv2.morphologyEx(yellow_mask, cv2.MORPH_OPEN, kernel)

    blue_contours, _ = cv2.findContours(
        blue_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    if len(blue_contours) > 0:
        largest_blue = max(blue_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_blue) > MIN_CONTOUR_AREA:
            parallelogram = detect_parallelogram(largest_blue)

            if parallelogram is not None:
                # Instead of using drawContours, draw the lines individually:
                for i in range(4):
                    pt1 = tuple(parallelogram[i].astype(int))
                    pt2 = tuple(parallelogram[(i + 1) % 4].astype(int))
                    cv2.line(output, pt1, pt2, (255, 0, 0), 2)

                # Also draw the points for debugging
                for i, pt in enumerate(parallelogram):
                    cv2.circle(output, tuple(pt.astype(int)), 5, (0, 0, 255), -1)

                # Find bottom edge points (the two with largest y values)
                bottom_points = sorted(parallelogram, key=lambda p: p[1], reverse=True)[
                    :2
                ]
                bottom_points = sorted(bottom_points, key=lambda p: p[0])  # Sort by x

                # Get center of bottom edge
                bottom_center_x = int((bottom_points[0][0] + bottom_points[1][0]) / 2)
                bottom_center_y = int((bottom_points[0][1] + bottom_points[1][1]) / 2)

                # Draw bottom center
                cv2.circle(
                    output, (bottom_center_x, bottom_center_y), 5, (255, 255, 0), -1
                )
                cv2.putText(
                    output,
                    f"({bottom_center_x},{bottom_center_y})",
                    (bottom_center_x - 40, bottom_center_y - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.5,
                    (255, 255, 0),
                    1,
                )
            else:
                x, y, w, h = cv2.boundingRect(largest_blue)
                if h > MIN_GOAL_HEIGHT:
                    # Find true bottom edge
                    bottom_y = find_true_bottom_edge(
                        frame, (x, y, w, h), BLUE_LOWER, BLUE_UPPER
                    )
                    center_x = x + w // 2

                    # Draw only the bottom center point, not the rectangle
                    cv2.circle(output, (center_x, bottom_y), 5, (255, 255, 0), -1)
                    cv2.line(
                        output,
                        (center_x - 10, bottom_y),
                        (center_x + 10, bottom_y),
                        (255, 255, 0),
                        2,
                    )
                    cv2.putText(
                        output,
                        f"({center_x},{bottom_y})",
                        (center_x - 40, bottom_y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.5,
                        (255, 255, 0),
                        1,
                    )

    yellow_contours, _ = cv2.findContours(
        yellow_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    if len(yellow_contours) > 0:
        largest_yellow = max(yellow_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_yellow) > MIN_CONTOUR_AREA:
            parallelogram = detect_parallelogram(largest_yellow)

            if parallelogram is not None:
                # Instead of using drawContours, draw the lines individually:
                for i in range(4):
                    pt1 = tuple(parallelogram[i].astype(int))
                    pt2 = tuple(parallelogram[(i + 1) % 4].astype(int))
                    cv2.line(output, pt1, pt2, (0, 255, 255), 2)

                # Also draw the points for debugging
                for i, pt in enumerate(parallelogram):
                    cv2.circle(output, tuple(pt.astype(int)), 5, (0, 0, 255), -1)

                # Find bottom edge points (the two with largest y values)
                bottom_points = sorted(parallelogram, key=lambda p: p[1], reverse=True)[
                    :2
                ]
                bottom_points = sorted(bottom_points, key=lambda p: p[0])  # Sort by x

                # Get center of bottom edge
                bottom_center_x = int((bottom_points[0][0] + bottom_points[1][0]) / 2)
                bottom_center_y = int((bottom_points[0][1] + bottom_points[1][1]) / 2)

                # Draw bottom center
                cv2.circle(
                    output, (bottom_center_x, bottom_center_y), 5, (0, 255, 255), -1
                )
                cv2.putText(
                    output,
                    f"({bottom_center_x},{bottom_center_y})",
                    (bottom_center_x - 40, bottom_center_y - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.5,
                    (0, 255, 255),
                    1,
                )
            else:
                x, y, w, h = cv2.boundingRect(largest_yellow)
                if h > MIN_GOAL_HEIGHT:
                    bottom_y = find_true_bottom_edge(
                        frame, (x, y, w, h), YELLOW_LOWER, YELLOW_UPPER
                    )
                    center_x = x + w // 2

                    # Draw only the bottom center point, not the rectangle
                    cv2.circle(output, (center_x, bottom_y), 5, (0, 255, 255), -1)
                    cv2.line(
                        output,
                        (center_x - 10, bottom_y),
                        (center_x + 10, bottom_y),
                        (0, 255, 255),
                        2,
                    )
                    cv2.putText(
                        output,
                        f"({center_x},{bottom_y})",
                        (center_x - 40, bottom_y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.5,
                        (0, 255, 255),
                        1,
                    )

    cv2.imshow("Goal Detection", output)
    
    if cv2.waitKey(25) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()
