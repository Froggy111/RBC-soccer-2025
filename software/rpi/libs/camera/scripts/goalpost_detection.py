import cv2
import numpy as np
import os
import datetime
import math
import time

DEBUG = True

video_path = "vid4.mp4"
print("file exists? " + str(os.path.exists("vid4.mp4")))
cap = cv2.VideoCapture(video_path)

# Correct HSV values for blue
# (OpenCV uses H:0-180, S:0-255, V:0-255)
BLUE_LOWER = np.array([100, 50, 50])  # Blue around 100-130 hue
BLUE_UPPER = np.array([150, 255, 255])  # Upper limit within valid range

# Correct HSV values for yellow
YELLOW_LOWER = np.array([20, 137, 110])  # Yellow is around 20-30 in OpenCV HSV
YELLOW_UPPER = np.array([30, 255, 255])

# Field green
FIELD_LOWER = np.array([35, 50, 50])  # Green is around 35-85 in OpenCV HSV
FIELD_UPPER = np.array([85, 255, 255])

MIN_CONTOUR_AREA = 400
MIN_GOAL_HEIGHT = 10
EDGE_SEARCH_HEIGHT = 69
EPSILON_FACTOR = 0.05
PARALLELOGRAM_TOLERANCE = 1
CENTRE_POINT = [640 // 2 - 5, 480 // 2 + 30]


def find_true_bottom_edge(frame, bbox, goal_lower, goal_upper):
    """Find the actual bottom edge by detecting the field-goal transition."""
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
    """
    Detect the field-goal transition for a parallelogram shape
    and return the actual bottom edge.
    """
    x, y, w, h = cv2.boundingRect(parallelogram.reshape(-1, 1, 2))
    roi_height = min(h + EDGE_SEARCH_HEIGHT, frame.shape[0] - y)
    roi = frame[y : y + roi_height, x : x + w]

    mask = np.zeros((roi_height, w), dtype=np.uint8)
    shifted_parallelogram = parallelogram - np.array([x, y])
    cv2.fillPoly(mask, [shifted_parallelogram.reshape(-1, 1, 2)], 255)

    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    goal_mask = cv2.inRange(hsv, goal_lower, goal_upper) & mask
    field_mask = cv2.inRange(hsv, FIELD_LOWER, FIELD_UPPER) & mask
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


def find_true_bottom_edge_quadrilateral(frame, quadrilateral, goal_lower, goal_upper):
    """
    Detect the field-goal transition for a quadrilateral shape
    and return the actual bottom edge.
    """
    x, y, w, h = cv2.boundingRect(quadrilateral.reshape(-1, 1, 2))
    roi_height = min(h + EDGE_SEARCH_HEIGHT, frame.shape[0] - y)
    roi = frame[y : y + roi_height, x : x + w]

    mask = np.zeros((roi_height, w), dtype=np.uint8)
    shifted_quad = quadrilateral - np.array([x, y])
    shifted_quad = np.clip(shifted_quad, [0, 0], [w - 1, roi_height - 1])
    shifted_quad_reshaped = shifted_quad.reshape((-1, 1, 2)).astype(np.int32)
    cv2.fillPoly(mask, [shifted_quad_reshaped], 255)

    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    goal_mask = cv2.inRange(hsv, goal_lower, goal_upper) & mask
    field_mask = cv2.inRange(hsv, FIELD_LOWER, FIELD_UPPER) & mask

    vertical_proj = np.sum(goal_mask, axis=1)
    threshold = 0.2 * np.max(vertical_proj) if len(vertical_proj) > 0 else 0
    goal_rows = np.where(vertical_proj > threshold)[0]
    if len(goal_rows) == 0:
        return y + h

    bottom_row = goal_rows[-1]
    for row in range(bottom_row, min(bottom_row + EDGE_SEARCH_HEIGHT, roi_height)):
        if row >= field_mask.shape[0]:
            break
        if np.sum(field_mask[row, :]) > 0.05 * w * 255:
            bottom_row = row
            break

    return y + bottom_row


def order_points(pts):
    """
    Return points in the order [top-left, top-right, bottom-right, bottom-left].
    """
    rect = np.zeros((4, 2), dtype="float32")
    s = pts.sum(axis=1)
    rect[0] = pts[np.argmin(s)]
    rect[2] = pts[np.argmax(s)]

    diff = np.diff(pts, axis=1)
    rect[1] = pts[np.argmin(diff)]
    rect[3] = pts[np.argmax(diff)]
    return rect


def detect_quadrilateral(
    contour, epsilon_factor=0.03, min_height=MIN_GOAL_HEIGHT, max_points=6
):
    """
    Detect a quadrilateral from a given contour with flexible criteria.
    Return the 4 corner points if found, otherwise return None.
    """
    perimeter = cv2.arcLength(contour, True)
    epsilon = epsilon_factor * perimeter
    approx = cv2.approxPolyDP(contour, epsilon, True)

    if DEBUG:
        print(f"Contour has {len(approx)} points")

    if len(approx) < 4 or len(approx) > max_points:
        if DEBUG:
            print(f"Skipping shape with {len(approx)} points")
        return None

    if len(approx) > 4:
        center = np.mean(approx.reshape(-1, 2), axis=0)
        points = approx.reshape(-1, 2)
        distances = np.sqrt(np.sum((points - center) ** 2, axis=1))
        indices = np.argsort(distances)[-4:]
        pts = points[indices]
    else:
        pts = np.array([point[0] for point in approx])

    x, y, w, h = cv2.boundingRect(pts.reshape(-1, 1, 2))
    if h < min_height or w < 5 or (h / w) > 8.0:
        if DEBUG:
            print(f"Skipping shape with h/w ratio {h/w}")
        return None

    if len(pts) == 4:
        pts = order_quadrilateral_points(pts)

    return pts


def order_quadrilateral_points(pts):
    """
    Order points of a quadrilateral in [top-left, top-right, bottom-right, bottom-left] order.
    """
    x_sorted = pts[np.argsort(pts[:, 0])]
    left_points = x_sorted[:2]
    right_points = x_sorted[2:]

    left_points = left_points[np.argsort(left_points[:, 1])]
    right_points = right_points[np.argsort(right_points[:, 1])]

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
    Find the point of the quadrilateral closest to the bottom of the image
    (the robot), near the detected field transition line.
    """
    bot_center_x = frame_height // 2
    bot_center_y = frame_height
    field_threshold = 20
    field_points = []

    for point in quadrilateral:
        if abs(point[1] - bottom_edge_y) < field_threshold:
            field_points.append(point)

    if not field_points:
        field_points = quadrilateral

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
    """
    Return the most reliable contour for a goalpost.
    If no contours exist, return None.
    """
    if not contours:
        return None

    sorted_contours = sorted(contours, key=cv2.contourArea, reverse=True)
    largest = sorted_contours[0]
    if cv2.contourArea(largest) < MIN_CONTOUR_AREA:
        return None

    hull = cv2.convexHull(largest)
    x, y, w, h = cv2.boundingRect(hull)
    aspect_ratio = h / max(w, 1)

    if aspect_ratio > 1.5:
        return hull
    return largest


def filter_out_rods(contours):
    """
    Filter out thin rod-like contours (very tall and thin).
    """
    filtered_contours = []
    for contour in contours:
        rect = cv2.minAreaRect(contour)
        width, height = rect[1]

        if width > height:
            width, height = height, width

        aspect_ratio = height / max(width, 1)
        if aspect_ratio > 8.0:
            continue

        filtered_contours.append(contour)

    return filtered_contours


def combine_goalpost_parts(contours, min_area):
    """
    Combine multiple significant contours by creating a convex hull from them.
    """
    significant_contours = [c for c in contours if cv2.contourArea(c) > min_area / 4]
    if len(significant_contours) > 1:
        all_points = np.vstack([c.reshape(-1, 2) for c in significant_contours])
        return cv2.convexHull(all_points.reshape(-1, 1, 2))
    elif len(significant_contours) == 1:
        return significant_contours[0]
    return None


def find_line_midpoint(line_points):
    """
    Find the midpoint of two line-points.
    """
    total_x = (line_points[0][0] + line_points[1][0]) / 2
    total_y = (line_points[0][1] + line_points[1][1]) / 2
    return [total_x, total_y]


def find_quad_midpoint(quad_points):
    """
    Find the midpoint of four quadrilateral points.
    """
    total_x = 0.0
    total_y = 0.0
    for i in range(4):
        total_x += quad_points[i][0]
        total_y += quad_points[i][1]
    return [total_x / 4, total_y / 4]


def find_goal_angle(goal_midpoint):
    """
    Convert the midpoint location into an angle relative to CENTRE_POINT.
    """
    delta_x = goal_midpoint[0].astype(int) - CENTRE_POINT[0]
    delta_y = goal_midpoint[1].astype(int) - CENTRE_POINT[1]
    goal_angle = math.atan2(delta_y, delta_x)
    return goal_angle + math.pi / 2


def find_closest_points_to_center(quad_points, center_point=CENTRE_POINT):
    """
    Find the two points of the quadrilateral closest to the center_point.
    """
    distances = []
    for i, point in enumerate(quad_points):
        dist = np.sqrt(
            (point[0] - center_point[0]) ** 2 + (point[1] - center_point[1]) ** 2
        )
        distances.append((i, dist))

    distances.sort(key=lambda x: x[1])
    closest_indices = [distances[0][0], distances[1][0]]
    return quad_points[closest_indices[0]], quad_points[closest_indices[1]]

def find_distance_to_goal(goal_midpoint): #goal_midpoint is a 2-element list
    x_delta = goal_midpoint[0] - CENTRE_POINT[0]
    y_delta = goal_midpoint[1] - CENTRE_POINT[1]

    return (x_delta**2 + y_delta**2)**0.5


# Main loop
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    start_time = datetime.datetime.now()
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    output = frame.copy()

    if DEBUG:
        debug_mask = np.zeros_like(frame)

    # Step 1: Create and clean color masks
    blue_mask = cv2.inRange(hsv, BLUE_LOWER, BLUE_UPPER)
    yellow_mask = cv2.inRange(hsv, YELLOW_LOWER, YELLOW_UPPER)

    # Step 2: Apply morphology (minimal)
    kernel_small = np.ones((3, 3), np.uint8)
    blue_mask = cv2.morphologyEx(blue_mask, cv2.MORPH_CLOSE, kernel_small)
    yellow_mask = cv2.morphologyEx(yellow_mask, cv2.MORPH_CLOSE, kernel_small)

    # Step 3: Find contours
    blue_contours, _ = cv2.findContours(
        blue_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )
    yellow_contours, _ = cv2.findContours(
        yellow_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    # Step 4: Process blue goalpost
    blue_detected = False
    cv2.circle(output, (640 // 2 - 5, 480 // 2 + 30), 5, (255, 255, 255), -1)

    filtered_blue = filter_out_rods(blue_contours)
    if len(filtered_blue) > 1:
        combined_blue = combine_goalpost_parts(filtered_blue, MIN_CONTOUR_AREA / 2)
        if combined_blue is not None:
            quad = detect_quadrilateral(combined_blue, EPSILON_FACTOR)
            if quad is not None:
                blue_detected = True
                blue_quad = quad

    if not blue_detected and filtered_blue:
        reliable_blue = get_reliable_goalpost_contour(filtered_blue, blue_mask)
        if reliable_blue is not None:
            quad = detect_quadrilateral(reliable_blue, EPSILON_FACTOR)
            if quad is not None:
                blue_detected = True
                blue_quad = quad

    if not blue_detected and blue_contours:
        largest_blue = max(blue_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_blue) > MIN_CONTOUR_AREA:
            quad = detect_quadrilateral(largest_blue, EPSILON_FACTOR)
            if quad is not None:
                blue_detected = True
                blue_quad = quad

    if blue_detected:
        for i in range(4):
            pt1 = tuple(blue_quad[i].astype(int))
            pt2 = tuple(blue_quad[(i + 1) % 4].astype(int))
            cv2.line(output, pt1, pt2, (255, 0, 0), 2)

        closest_point1, closest_point2 = find_closest_points_to_center(blue_quad)
        point1_int = tuple(closest_point1.astype(int))
        point2_int = tuple(closest_point2.astype(int))
        blue_bottom_midpoint = find_line_midpoint(
            [closest_point1.astype(int), closest_point2.astype(int)]
        )
        blue_goal_distance = find_distance_to_goal(blue_bottom_midpoint)
        print("Distance to Blue Goal: " + str(blue_goal_distance))
        cv2.circle(output, point1_int, 7, (255, 0, 0), -1)
        cv2.circle(output, point2_int, 7, (255, 0, 0), -1)
        cv2.line(output, point1_int, point2_int, (255, 0, 0), 2)

        blue_line_midpoint_x = int(blue_bottom_midpoint[0])
        blue_line_midpoint_y = int(blue_bottom_midpoint[1])
        cv2.circle(
            output, (blue_line_midpoint_x, blue_line_midpoint_y), 7, (255, 255, 0), -1
        )

        cv2.circle(output, point1_int, 7, (0, 255, 255), -1)
        cv2.circle(output, point2_int, 7, (0, 255, 255), -1)
        cv2.line(output, point1_int, point2_int, (0, 255, 255), 2)

        print(f"Closest points to center: {point1_int}, {point2_int}")

        bottom_edge_y = find_true_bottom_edge_quadrilateral(
            frame, blue_quad, BLUE_LOWER, BLUE_UPPER
        )
        nearest_point = find_nearest_field_point(
            blue_quad, bottom_edge_y, frame.shape[0]
        )
        nearest_x, nearest_y = nearest_point.astype(int)
        blue_midpoint = find_quad_midpoint(blue_quad)
        print(blue_midpoint)

        cv2.circle(output, (nearest_x, nearest_y), 5, (255, 255, 0), -1)
        cv2.circle(
            output,
            (blue_midpoint[0].astype(int), blue_midpoint[1].astype(int)),
            5,
            (255, 255, 255),
            -1,
        )
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
        blue_goal_angle = find_goal_angle(blue_midpoint)
        cv2.putText(
            output,
            f"Angle: {(blue_goal_angle * 180 / (math.pi)):.1f} deg",
            (blue_midpoint[0].astype(int) - 40, blue_midpoint[1].astype(int) + 40),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (0, 0, 255),
            2,
        )

    # Step 5: Process yellow goalpost
    yellow_detected = False
    filtered_yellow = filter_out_rods(yellow_contours)
    if len(filtered_yellow) > 1:
        combined_yellow = combine_goalpost_parts(filtered_yellow, MIN_CONTOUR_AREA / 2)
        if combined_yellow is not None:
            quad = detect_quadrilateral(combined_yellow, EPSILON_FACTOR)
            if quad is not None:
                yellow_detected = True
                yellow_quad = quad

    if not yellow_detected and filtered_yellow:
        reliable_yellow = get_reliable_goalpost_contour(filtered_yellow, yellow_mask)
        if reliable_yellow is not None:
            quad = detect_quadrilateral(reliable_yellow, EPSILON_FACTOR)
            if quad is not None:
                yellow_detected = True
                yellow_quad = quad

    if not yellow_detected and yellow_contours:
        largest_yellow = max(yellow_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_yellow) > MIN_CONTOUR_AREA:
            quad = detect_quadrilateral(largest_yellow, EPSILON_FACTOR)
            if quad is not None:
                yellow_detected = True
                yellow_quad = quad

    if yellow_detected:
        for i in range(4):
            pt1 = tuple(yellow_quad[i].astype(int))
            pt2 = tuple(yellow_quad[(i + 1) % 4].astype(int))
            cv2.line(output, pt1, pt2, (0, 255, 255), 2)

        closest_point1, closest_point2 = find_closest_points_to_center(yellow_quad)
        yellow_bottom_midpoint = find_line_midpoint(
            [closest_point1.astype(int), closest_point2.astype(int)]
        )
        yellow_goal_distance = find_distance_to_goal(yellow_bottom_midpoint)
        print("Distance to Yellow Goal: " + str(yellow_goal_distance))
        point1_int = tuple(closest_point1.astype(int))
        point2_int = tuple(closest_point2.astype(int))

        cv2.circle(output, point1_int, 7, (255, 0, 0), -1)
        cv2.circle(output, point2_int, 7, (255, 0, 0), -1)
        cv2.line(output, point1_int, point2_int, (255, 0, 0), 2)

        yellow_line_midpoint_x = int(yellow_bottom_midpoint[0])
        yellow_line_midpoint_y = int(yellow_bottom_midpoint[1])
        cv2.circle(
            output, (yellow_line_midpoint_x, yellow_line_midpoint_y), 7, (0, 0, 255), -1
        )

        print(f"Yellow closest points to center: {point1_int}, {point2_int}")

        bottom_edge_y = find_true_bottom_edge_quadrilateral(
            frame, yellow_quad, YELLOW_LOWER, YELLOW_UPPER
        )
        nearest_point = find_nearest_field_point(
            yellow_quad, bottom_edge_y, frame.shape[0]
        )
        yellow_midpoint = find_quad_midpoint(yellow_quad)
        nearest_x, nearest_y = nearest_point.astype(int)

        cv2.circle(output, (nearest_x, nearest_y), 5, (0, 255, 255), -1)
        cv2.circle(
            output,
            (yellow_midpoint[0].astype(int), yellow_midpoint[1].astype(int)),
            5,
            (255, 255, 255),
            -1,
        )
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
        yellow_goal_angle = find_goal_angle(yellow_midpoint)
        cv2.putText(
            output,
            f"Angle: {(yellow_goal_angle * 180 / (math.pi)):.1f} deg",
            (yellow_midpoint[0].astype(int) - 40, yellow_midpoint[1].astype(int) + 40),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (0, 0, 255),
            2,
        )

    if DEBUG:
        blue_display = cv2.cvtColor(blue_mask, cv2.COLOR_GRAY2BGR)
        yellow_display = cv2.cvtColor(yellow_mask, cv2.COLOR_GRAY2BGR)
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

        h, w = frame.shape[:2]
        blue_display = cv2.resize(blue_display, (w // 2, h // 2))
        yellow_display = cv2.resize(yellow_display, (w // 2, h // 2))

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
