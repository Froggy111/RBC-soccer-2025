import cv2
import numpy as np
import os
import datetime
import time
import math

DEBUG = True

video_path = "RBC-soccer-2025/software/rpi-direct/libs/camera/scripts/vid.mp4"
print(
    "file exists? "
    + str(os.path.exists("RBC-soccer-2025/software/rpi-direct/libs/camera/scripts/vid.mp4"))
)
cap = cv2.VideoCapture(video_path)

# Correct HSV values for blue (OpenCV uses H:0-180, S:0-255, V:0-255)
BLUE_LOWER = np.array([70, 146, 50])  # Blue color has ~100-130 hue in OpenCV
BLUE_UPPER = np.array([150, 255, 255])  # Upper limit within valid range

# Correct HSV values for yellow
YELLOW_LOWER = np.array([20, 100, 100])  # Yellow is around 20-30 in OpenCV HSV
YELLOW_UPPER = np.array([30, 255, 255])

# Field green
FIELD_LOWER = np.array([35, 50, 50])  # Green is around 35-85 in OpenCV HSV
FIELD_UPPER = np.array([85, 255, 255])

MIN_CONTOUR_AREA = 300  # Reduced to detect smaller goalposts
MIN_GOAL_HEIGHT = 10
EDGE_SEARCH_HEIGHT = 69
EPSILON_FACTOR = 0.05  # Increased for better approximation
PARALLELOGRAM_TOLERANCE = 1  # Increased tolerance for parallelism


def find_true_bottom_edge(frame, bbox, goal_lower, goal_upper):
    """Finds the actual bottom edge by detecting field-goal transition"""
    x, y, w, h = bbox

    roi_height = min(h + EDGE_SEARCH_HEIGHT, frame.shape[0] - y)
    roi = frame[y : y + roi_height, x : x + w]

    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    goal_mask = cv2.inRange(hsv, goal_lower, goal_upper)
    field_mask = cv2.inRange(hsv, FIELD_LOWER, FIELD_UPPER)

    vertical_proj = np.sum(goal_mask, axis=1)

    threshold = 0.4 * np.max(vertical_proj) if len(vertical_proj) > 0 else 0
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


def find_true_bottom_edge_quadrilateral(frame, quadrilateral, goal_lower, goal_upper):
    """Finds the actual bottom edge by detecting field-goal transition for a quadrilateral"""
    # Get bounding box of the quadrilateral
    x, y, w, h = cv2.boundingRect(quadrilateral.reshape(-1, 1, 2))

    # Region of interest extends below the bounding box to find field transition
    roi_height = min(h + EDGE_SEARCH_HEIGHT, frame.shape[0] - y)
    roi = frame[y : y + roi_height, x : x + w]

    # Create a mask from the quadrilateral for more precise ROI
    mask = np.zeros((roi_height, w), dtype=np.uint8)
    shifted_quad = quadrilateral - np.array([x, y])  # Shift to ROI coordinates

    # Ensure points are within the mask bounds
    shifted_quad = np.clip(shifted_quad, [0, 0], [w - 1, roi_height - 1])

    # Create polygon mask
    shifted_quad_reshaped = shifted_quad.reshape((-1, 1, 2)).astype(np.int32)
    cv2.fillPoly(mask, [shifted_quad_reshaped], 255)

    # Apply color detection
    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    goal_mask = cv2.inRange(hsv, goal_lower, goal_upper) & mask
    field_mask = cv2.inRange(hsv, FIELD_LOWER, FIELD_UPPER) & mask

    # Find vertical projection
    vertical_proj = np.sum(goal_mask, axis=1)

    # Find bottom edge of goalpost
    threshold = 0.2 * np.max(vertical_proj) if len(vertical_proj) > 0 else 0
    goal_rows = np.where(vertical_proj > threshold)[0]

    if len(goal_rows) == 0:
        return y + h

    # Start from the last row of the goal and look for field
    bottom_row = goal_rows[-1]
    for row in range(bottom_row, min(bottom_row + EDGE_SEARCH_HEIGHT, roi_height)):
        if row >= field_mask.shape[0]:
            break
        if np.sum(field_mask[row, :]) > 0.05 * w * 255:
            bottom_row = row
            break

    return y + bottom_row


def order_points(pts):
    """Order points in [top-left, top-right, bottom-right, bottom-left] order"""
    # Create a rectangular bounding box
    rect = np.zeros((4, 2), dtype="float32")

    # Get sum of points (top-left has smallest sum, bottom-right has largest)
    s = pts.sum(axis=1)
    rect[0] = pts[np.argmin(s)]  # Top-left
    rect[2] = pts[np.argmax(s)]  # Bottom-right

    # Get diff of points (top-right has smallest diff, bottom-left has largest)
    diff = np.diff(pts, axis=1)
    rect[1] = pts[np.argmin(diff)]  # Top-right
    rect[3] = pts[np.argmax(diff)]  # Bottom-left

    return rect


def detect_quadrilateral(
    contour, epsilon_factor=0.03, min_height=MIN_GOAL_HEIGHT, max_points=6
):
    """
    Detect any quadrilateral from contour with flexible criteria
    Returns the 4 corner points of the quadrilateral if found, None otherwise
    """
    # Approximate the contour
    perimeter = cv2.arcLength(contour, True)
    epsilon = epsilon_factor * perimeter
    approx = cv2.approxPolyDP(contour, epsilon, True)

    # For debugging
    if DEBUG:
        print(f"Contour has {len(approx)} points")

    # More flexible point count check - many real-world "quadrilaterals"
    # might be approximated with 3-6 points depending on noise and perspective
    if len(approx) < 4 or len(approx) > max_points:
        if DEBUG:
            print(f"Skipping shape with {len(approx)} points")
        return None

    # If we have more than 4 points, simplify to get the most significant 4
    if len(approx) > 4:
        # Sort points by their distance from center to get the most extreme points
        center = np.mean(approx.reshape(-1, 2), axis=0)
        points = approx.reshape(-1, 2)
        distances = np.sqrt(np.sum((points - center) ** 2, axis=1))
        # Get indices of the 4 points furthest from center
        indices = np.argsort(distances)[-4:]
        pts = points[indices]
    else:
        # Use all points if we have 4 or fewer
        pts = np.array([point[0] for point in approx])

    # Basic size check
    x, y, w, h = cv2.boundingRect(pts.reshape(-1, 1, 2))
    if h < min_height or w < 5 or h / w > 8.0:  # Avoid very thin rectangles
        if DEBUG:
            print(f"Skipping shape with h/w ratio {h/w}")
        return None

    # For quadrilaterals, order the points for consistent results
    # (top-left, top-right, bottom-right, bottom-left)
    if len(pts) == 4:
        pts = order_quadrilateral_points(pts)

    return pts


def order_quadrilateral_points(pts):
    """Order points in [top-left, top-right, bottom-right, bottom-left] order"""
    # Sort by x coordinate
    x_sorted = pts[np.argsort(pts[:, 0])]

    # Get left points and right points
    left_points = x_sorted[:2]
    right_points = x_sorted[2:]

    # Sort left points by y
    left_points = left_points[np.argsort(left_points[:, 1])]
    # Sort right points by y
    right_points = right_points[np.argsort(right_points[:, 1])]

    # Return ordered points
    return np.array(
        [
            left_points[0],  # top-left
            right_points[0],  # top-right
            right_points[1],  # bottom-right
            left_points[1],  # bottom-left
        ]
    )


def find_nearest_field_point(quadrilateral, bottom_edge_y, frame_height):
    """
    Find the point of the quadrilateral that is:
    1. Closest to the bottom of the image (nearest to the robot)
    2. Also respecting the field transition
    """
    # Get center bottom point of the image (representing robot position)
    bot_center_x = frame_height // 2
    bot_center_y = frame_height  # Bottom of image

    # Filter points that are near the detected field transition line
    field_threshold = 20  # Allow points within 20 pixels of the field line
    field_points = []

    for point in quadrilateral:
        # If point is within threshold of the field line
        if abs(point[1] - bottom_edge_y) < field_threshold:
            field_points.append(point)

    # If no points are near the field line, use all points
    if not field_points:
        field_points = quadrilateral

    # Now find the point closest to the robot (bottom center of image)
    closest_point = None
    min_distance = float("inf")

    for point in field_points:
        distance = np.sqrt(
            (point[0] - bot_center_x) ** 2 + (point[1] - bot_center_y) ** 2
        )
        if distance < min_distance:
            min_distance = distance
            closest_point = point

    return closest_point


def get_reliable_goalpost_contour(contours, color_mask):
    """Get the most reliable contour for the goalpost"""
    if not contours:
        return None

    # Sort by area, largest first
    sorted_contours = sorted(contours, key=cv2.contourArea, reverse=True)
    largest = sorted_contours[0]

    # Check if it's large enough
    if cv2.contourArea(largest) < MIN_CONTOUR_AREA:
        return None

    # Create a convex hull around the largest contour to smooth it
    hull = cv2.convexHull(largest)

    # Calculate aspect ratio of hull
    x, y, w, h = cv2.boundingRect(hull)
    aspect_ratio = h / max(w, 1)  # Avoid division by zero

    # If it's tall and narrow, it's likely a goalpost
    if aspect_ratio > 1.5:
        return hull

    return largest  # Fall back to largest if aspect ratio check fails


def filter_out_rods(contours):
    """Filter out thin rod-like contours"""
    filtered_contours = []

    for contour in contours:
        # Calculate minimum area rectangle
        rect = cv2.minAreaRect(contour)
        width, height = rect[1]

        # Ensure width and height are in correct order (width < height for vertical objects)
        if width > height:
            width, height = height, width

        # Calculate aspect ratio
        aspect_ratio = height / max(width, 1)

        # If it's too tall and thin, it's likely a rod (adjust threshold as needed)
        if aspect_ratio > 8.0:  # Rods are typically very tall and thin
            continue

        filtered_contours.append(contour)

    return filtered_contours


# If multiple significant contours, try combining them
def combine_goalpost_parts(contours, min_area):
    significant_contours = [c for c in contours if cv2.contourArea(c) > min_area / 4]
    if len(significant_contours) > 1:
        # Combine all points and create a convex hull
        all_points = np.vstack([c.reshape(-1, 2) for c in significant_contours])
        return cv2.convexHull(all_points.reshape(-1, 1, 2))
    elif len(significant_contours) == 1:
        return significant_contours[0]
    return None

def find_quad_midpoint(quad_points):
    total_x = 0.0
    total_y = 0.0
    for i in range(4):
        total_x += quad_points[i][0]
        total_y += quad_points[i][1]

    return [total_x/4, total_y/4]

def estimate_heading(blue_midpoint, yellow_midpoint):
    width = 640
    height = 480

    blue_est_heading = [10000.0, 10000.0]
    yellow_est_heading = [10000.0, 10000.0]

    if blue_midpoint[0].astype(int) < 1000:
        if blue_midpoint[0].astype(int) <= width/2:
            blue_est_heading = [0.0, math.pi]
            if blue_midpoint[1].astype(int) <= height/2:
                blue_est_heading[1] = math.pi/2
                if blue_midpoint[0].astype(int) > blue_midpoint[1].astype(int):
                    blue_est_heading[1] = math.pi/4
                else:
                    blue_est_heading[0] = math.pi/4
            else:
                blue_est_heading = [math.pi/2, math.pi]
            
                if width/2 - blue_midpoint[0].astype(int) > blue_midpoint[1].astype(int) - height/2:
                    blue_est_heading[1] = math.pi*3/4
                else:
                    blue_est_heading[0] = math.pi*3/4
        else:
            blue_est_heading = [-math.pi, 0.0]
            if blue_midpoint[1].astype(int) <= height/2:
                blue_est_heading[0] = -math.pi/2
                if blue_midpoint[0].astype(int) -  width/2 > height/2 - blue_midpoint[1].astype(int):
                    blue_est_heading[1] = -math.pi/4
                else:
                    blue_est_heading[0] = -math.pi/4
            else:
                blue_est_heading = [-math.pi, -math.pi/2]
                if blue_midpoint[0].astype(int) - width/2 > blue_midpoint[1].astype(int) - height/2:
                    blue_est_heading[0] = -math.pi*3/4
                else:

                    blue_est_heading[1] = -math.pi*3/4

    if yellow_midpoint[0].astype(int) < 1000:
        if yellow_midpoint[0].astype(int) <= width/2:
            yellow_est_heading = [-math.pi, 0.0]
            if yellow_midpoint[1].astype(int) <= height/2:
                yellow_est_heading[1] = -math.pi/2
                if yellow_midpoint[0].astype(int) > yellow_midpoint[1].astype(int):
                    yellow_est_heading[1] = -math.pi*3/4
                else:
                    yellow_est_heading[0] = -math.pi*3/4
            else:
                yellow_est_heading = [-math.pi/2, 0.0] #90 - 180
            
                if width/2 - yellow_midpoint[0].astype(int) > yellow_midpoint[1].astype(int) - height/2:
                    yellow_est_heading[1] = -math.pi/4
                else:
                    yellow_est_heading[0] = -math.pi/4
        else:
            yellow_est_heading[1] = [0.0, math.pi]
            if yellow_midpoint[1].astype(int) <= height/2:
                yellow_est_heading[0] = math.pi/2
                if yellow_midpoint[0].astype(int) -  width/2 > height/2 - yellow_midpoint[1].astype(int):
                    yellow_est_heading[1] = math.pi*3/4
                else:
                    yellow_est_heading[0] = math.pi*3/4
            else:
                yellow_est_heading = [0.0, math.pi/2]
                if yellow_midpoint[0].astype(int) - width/2 > yellow_midpoint[1].astype(int) - height/2:
                    yellow_est_heading[1] = math.pi*3/4
                else:
                    yellow_est_heading[0] = math.pi*3.4

    est_heading = 0.0   
    count = 0
    if blue_est_heading[1] != 10000.0: 
        est_heading += blue_est_heading[1]
        count += 1
    if yellow_est_heading[0] != 10000.0:
        est_heading += yellow_est_heading[0]
        count += 1
    
    if count > 0:
        est_heading /= count
    
    return est_heading

def estimate_heading_new(blue_midpoint, yellow_midpoint):
    width = 640
    height = 480

    blue_vertical = 0
    blue_horizontal = 0

    yellow_vertical = 0
    yellow_horizontal = 0

    if blue_midpoint[0].astype(int) < 1000:
        if blue_midpoint[1].astype(int) <= height/2:
            blue_vertical = 1 #[-math.pi/2, math.pi/2]
        else:
            blue_vertical = 2 #[math.pi/2, math.pi*3/2]
        
        if blue_midpoint[0].astype(int) <= width/2:
            blue_horizontal = 3 #[0.0, math.pi]
        else:
            blue_horizontal = 4 #[-math.pi, 0.0]
    
    if yellow_midpoint[0].astype(int) < 1000:
        if yellow_midpoint[1].astype(int) <= height/2:
            yellow_vertical = 2 #[math.pi/2, math.pi*3/2]
        else:
            yellow_vertical = 1 #[-math.pi/2, math.pi/2]
        
        if yellow_midpoint[0].astype(int) <= width/2:
            yellow_horizontal = 4 #[-math.pi, 0.0]
        else:
            yellow_horizontal = 3 #[0.0, math.pi]

    
    blue_est_heading = 10000.0
    yellow_est_heading = 10000.0

    if blue_vertical == 1:
        if blue_horizontal == 3:
            blue_est_heading = math.pi/4
        else:
            blue_est_heading = -math.pi/4
    elif blue_vertical == 2:
        if blue_horizontal == 3:
            blue_est_heading = math.pi*3/4
        else:
            blue_est_heading = -math.pi*3/4

    if yellow_vertical == 1:
        if yellow_horizontal == 3:
            yellow_est_heading = math.pi/4
        else:
            yellow_est_heading = -math.pi/4
    elif yellow_vertical == 2:
        if yellow_horizontal == 3:
            yellow_est_heading = math.pi*3/4
        else:
            yellow_est_heading = -math.pi*3/4
   

    est_heading = 0.0
    count = 0
    if blue_est_heading != 10000.0: 
        est_heading += blue_est_heading
        count += 1
    if yellow_est_heading != 10000.0:
        est_heading += yellow_est_heading
        count += 1
    
    if count > 0:
        est_heading /= count
    
    return est_heading


# Main loop
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    start_time = datetime.datetime.now()

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    output = frame.copy()

    # Create debug image if in debug mode
    if DEBUG:
        debug_mask = np.zeros_like(frame)

    # Step 1: Create and clean color masks
    blue_mask = cv2.inRange(hsv, BLUE_LOWER, BLUE_UPPER)
    yellow_mask = cv2.inRange(hsv, YELLOW_LOWER, YELLOW_UPPER)

    # Step 2: Apply morphology - much gentler approach
    kernel_small = np.ones((3, 3), np.uint8)
    kernel = np.ones((5, 5), np.uint8)

    # Apply minimal morphology
    blue_mask = cv2.morphologyEx(blue_mask, cv2.MORPH_CLOSE, kernel_small)
    yellow_mask = cv2.morphologyEx(yellow_mask, cv2.MORPH_CLOSE, kernel_small)

    # Step 3: Find contours
    blue_contours, _ = cv2.findContours(
        blue_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )
    yellow_contours, _ = cv2.findContours(
        yellow_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    # Step 4: Process blue goalpost - try multiple approaches
    blue_detected = False

    # Try approach 1: Filter and combine
    filtered_blue = filter_out_rods(blue_contours)
    if len(filtered_blue) > 1:
        combined_blue = combine_goalpost_parts(filtered_blue, MIN_CONTOUR_AREA / 2)
        if combined_blue is not None:
            quad = detect_quadrilateral(combined_blue, EPSILON_FACTOR)
            if quad is not None:
                blue_detected = True
                blue_quad = quad

    # Try approach 2: Use reliable goalpost contour if approach 1 failed
    if not blue_detected and filtered_blue:
        reliable_blue = get_reliable_goalpost_contour(filtered_blue, blue_mask)
        if reliable_blue is not None:
            quad = detect_quadrilateral(reliable_blue, EPSILON_FACTOR)
            if quad is not None:
                blue_detected = True
                blue_quad = quad

    # Try approach 3: Use largest contour directly if all else fails
    if not blue_detected and blue_contours:
        largest_blue = max(blue_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_blue) > MIN_CONTOUR_AREA:
            quad = detect_quadrilateral(largest_blue, EPSILON_FACTOR)
            if quad is not None:
                blue_detected = True
                blue_quad = quad

    # Draw blue goalpost if detected
    if blue_detected:
        # Draw the quadrilateral
        for i in range(4):
            pt1 = tuple(blue_quad[i].astype(int))
            pt2 = tuple(blue_quad[(i + 1) % 4].astype(int))
            cv2.line(output, pt1, pt2, (255, 0, 0), 2)

        # Find the true bottom edge
        bottom_edge_y = find_true_bottom_edge_quadrilateral(
            frame, blue_quad, BLUE_LOWER, BLUE_UPPER
        )

        # Find the point nearest to the robot and closest to the field
        nearest_point = find_nearest_field_point(
            blue_quad, bottom_edge_y, frame.shape[0]
        )
        nearest_x, nearest_y = nearest_point.astype(int)
        blue_midpoint = find_quad_midpoint(blue_quad)
        print(blue_midpoint)
        # Draw the nearest field point
        cv2.circle(output, (nearest_x, nearest_y), 5, (255, 255, 0), -1)
        cv2.circle(output, (blue_midpoint[0].astype(int), blue_midpoint[1].astype(int)), 5, (255, 255, 255), -1)

        cv2.line(
            output,
            (nearest_x - 10, nearest_y),
            (nearest_x + 10, nearest_y),
            (255, 255, 0),
            2,
        )
        cv2.putText(
            output,
            f"({nearest_x},{nearest_y})",
            (nearest_x - 40, nearest_y - 10),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (255, 255, 0),
            1,
        )
        cv2.putText(
            output,
            f"({blue_midpoint[0].astype(int)},{blue_midpoint[1].astype(int)})",
            (blue_midpoint[0].astype(int) - 40, blue_midpoint[1].astype(int) - 10),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (255, 255, 255),
            1,
        )

    # Step 5: Process yellow goalpost - using same approach as blue
    yellow_detected = False

    # Try approach 1: Filter and combine
    filtered_yellow = filter_out_rods(yellow_contours)
    if len(filtered_yellow) > 1:
        combined_yellow = combine_goalpost_parts(filtered_yellow, MIN_CONTOUR_AREA / 2)
        if combined_yellow is not None:
            quad = detect_quadrilateral(combined_yellow, EPSILON_FACTOR)
            if quad is not None:
                yellow_detected = True
                yellow_quad = quad

    # Try approach 2: Use reliable goalpost contour if approach 1 failed
    if not yellow_detected and filtered_yellow:
        reliable_yellow = get_reliable_goalpost_contour(filtered_yellow, yellow_mask)
        if reliable_yellow is not None:
            quad = detect_quadrilateral(reliable_yellow, EPSILON_FACTOR)
            if quad is not None:
                yellow_detected = True
                yellow_quad = quad

    # Try approach 3: Use largest contour directly if all else fails
    if not yellow_detected and yellow_contours:
        largest_yellow = max(yellow_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_yellow) > MIN_CONTOUR_AREA:
            quad = detect_quadrilateral(largest_yellow, EPSILON_FACTOR)
            if quad is not None:
                yellow_detected = True
                yellow_quad = quad
    
    # Draw yellow goalpost if detected
    if yellow_detected:
        # Draw the quadrilateral
        for i in range(4):
            pt1 = tuple(yellow_quad[i].astype(int))
            pt2 = tuple(yellow_quad[(i + 1) % 4].astype(int))
            cv2.line(output, pt1, pt2, (0, 255, 255), 2)

        # Find the true bottom edge
        bottom_edge_y = find_true_bottom_edge_quadrilateral(
            frame, yellow_quad, YELLOW_LOWER, YELLOW_UPPER
        )

        # Find the point nearest to the robot and closest to the field
        nearest_point = find_nearest_field_point(
            yellow_quad, bottom_edge_y, frame.shape[0]
        )
        yellow_midpoint = find_quad_midpoint(yellow_quad)
        nearest_x, nearest_y = nearest_point.astype(int)

        # Draw the nearest field point
        cv2.circle(output, (nearest_x, nearest_y), 5, (0, 255, 255), -1)
        cv2.circle(output, (yellow_midpoint[0].astype(int), yellow_midpoint[1].astype(int)), 5, (255, 255, 255), -1)
        cv2.line(
            output,
            (nearest_x - 10, nearest_y),
            (nearest_x + 10, nearest_y),
            (0, 255, 255),
            2,
        )
        cv2.putText(
            output,
            f"({nearest_x},{nearest_y})",
            (nearest_x - 40, nearest_y - 10),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (0, 255, 255),
            1,
        )
        cv2.putText(
            output,
            f"({yellow_midpoint[0].astype(int)},{yellow_midpoint[1].astype(int)})",
            (yellow_midpoint[0].astype(int) - 40, yellow_midpoint[1].astype(int) - 10),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (255, 255, 255),
            1,
        )


    est_heading = 0.0
    if blue_detected and yellow_detected:
        est_heading = estimate_heading_new(blue_midpoint, yellow_midpoint)
    elif not blue_detected and not yellow_detected:
        pass
    elif not blue_detected:
        est_heading = estimate_heading_new(np.array([10000.0, 10000.0]), yellow_midpoint)
    else:
        est_heading = estimate_heading_new(blue_midpoint, np.array([10000.0, 10000.0]))
    
    
    cv2.putText(
        output,
        f"({(est_heading*180/(math.pi)):.2f})",
        (320, 240),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.5,
        (0, 255, 0),
        1,
    )
    # Display debug visualization
    if DEBUG:
        # Create colored versions of masks for display
        blue_display = cv2.cvtColor(blue_mask, cv2.COLOR_GRAY2BGR)
        yellow_display = cv2.cvtColor(yellow_mask, cv2.COLOR_GRAY2BGR)

        # Add text labels
        cv2.putText(
            blue_display,
            "Blue Mask",
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.7,
            (0, 255, 0),
            2,
        )
        cv2.putText(
            yellow_display,
            "Yellow Mask",
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.7,
            (0, 255, 0),
            2,
        )

        # Resize masks to half size
        h, w = frame.shape[:2]
        blue_display = cv2.resize(blue_display, (w // 2, h // 2))
        yellow_display = cv2.resize(yellow_display, (w // 2, h // 2))

        # Create a composite debug view
        debug_top = np.hstack([blue_display, yellow_display])
        debug_view = np.vstack([debug_top, cv2.resize(output, (w, h // 2))])

        cv2.imshow("Debug View", debug_view)

    cv2.imshow("Goal Detection", output)

    end_time = datetime.datetime.now()
    print("Time take for frame: " + str(end_time - start_time))

    if cv2.waitKey(25) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()
