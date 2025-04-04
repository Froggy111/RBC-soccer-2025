import cv2
import numpy as np
import os
import datetime
import math
import time

DEBUG = True
SAVE_POINTS = False  # Changed to False to disable CSV saving

video_path = "vid2.mp4"
print("file exists? " + str(os.path.exists("vid2.mp4")))
cap = cv2.VideoCapture(video_path)

# Constants for IR detection
MIN_CONTOUR_AREA = 10  # Much smaller than goalpost since we're looking for points
MIN_BRIGHTNESS = 130  # Lowered threshold for detecting bright spots
CENTRE_POINT = [640 // 2 - 5, 480 // 2 + 30]

# Adjusted Purple HSV range
PURPLE_LOWER = np.array([130, 50, 100])  # Expanded range to capture more variations
PURPLE_UPPER = np.array([175, 255, 255])


def calculate_distance(p1, p2):
    """Calculate Euclidean distance between two points."""
    return np.sqrt((p2[0] - p1[0]) ** 2 + (p2[1] - p1[1]) ** 2)


def calculate_angle(point, center=CENTRE_POINT):
    """Calculate angle from center to point (in radians)."""
    delta_x = point[0] - center[0]
    delta_y = point[1] - center[1]
    angle = math.atan2(delta_y, delta_x)
    return angle + math.pi / 2


def detect_ir_points(frame):
    """
    Detect bright purple (IR) points in the frame.
    Returns a list of all purple points in the current frame and the mask.
    """
    # Convert to HSV colorspace
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Create mask for purple color
    purple_mask = cv2.inRange(hsv, PURPLE_LOWER, PURPLE_UPPER)

    # Additionally check for brightness in original frame
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    bright_mask = cv2.threshold(gray, MIN_BRIGHTNESS, 255, cv2.THRESH_BINARY)[1]

    # Combine masks - we want purple AND bright
    ir_mask = cv2.bitwise_and(purple_mask, bright_mask)

    # Apply minimal morphology to clean up the mask
    kernel = np.ones((3, 3), np.uint8)
    ir_mask = cv2.morphologyEx(ir_mask, cv2.MORPH_OPEN, kernel)

    # Find contours in the mask
    contours, _ = cv2.findContours(ir_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Filter contours by minimum area
    ir_points = []
    for contour in contours:
        area = cv2.contourArea(contour)
        if area >= MIN_CONTOUR_AREA:
            # Calculate centroid of the contour
            M = cv2.moments(contour)
            if M["m00"] > 0:
                cx = int(M["m10"] / M["m00"])
                cy = int(M["m01"] / M["m00"])
                ir_points.append(
                    {
                        "position": (cx, cy),
                        "intensity": area,
                        "angle": calculate_angle((cx, cy)) * 180 / math.pi,
                        "distance": calculate_distance((cx, cy), CENTRE_POINT),
                    }
                )

    return ir_points, ir_mask


def find_strongest_ir_source(ir_points):
    """Find the strongest IR source based on intensity."""
    if not ir_points:
        return None

    # Sort by intensity (descending)
    sorted_points = sorted(ir_points, key=lambda p: p["intensity"], reverse=True)
    return sorted_points[0]["position"]


# For statistics - can be removed if not needed
all_frames_total_points = 0
frame_count = 0

# Main loop
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    frame_count += 1
    start_time = datetime.datetime.now()
    output = frame.copy()

    # Mark center point
    cv2.circle(output, tuple(CENTRE_POINT), 5, (255, 255, 255), -1)

    # Detect IR points - this returns all purple points in the current frame
    current_frame_points, ir_mask = detect_ir_points(frame)

    # Update statistics
    all_frames_total_points += len(current_frame_points)

    # Draw all detected IR points in current frame
    for point in current_frame_points:
        position = point["position"]
        intensity = point["intensity"]
        angle_degrees = point["angle"]
        distance = point["distance"]

        # Size of circle based on intensity
        radius = max(5, min(20, int(intensity / 10)))

        # Draw circle
        cv2.circle(output, position, radius, (255, 0, 255), -1)

        # Draw line from center to point
        cv2.line(output, tuple(CENTRE_POINT), position, (255, 0, 255), 2)

        # Display information
        cv2.putText(
            output,
            f"({position[0]},{position[1]})",
            (position[0] - 40, position[1] - 15),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (255, 0, 255),
            1,
        )
        cv2.putText(
            output,
            f"Angle: {angle_degrees:.1f}°",
            (position[0] - 40, position[1] + 15),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (255, 0, 255),
            1,
        )
        cv2.putText(
            output,
            f"Dist: {distance:.1f}",
            (position[0] - 40, position[1] + 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (255, 0, 255),
            1,
        )

    # Find and highlight the strongest IR source
    strongest_point = find_strongest_ir_source(current_frame_points)
    if strongest_point:
        cv2.circle(output, strongest_point, 15, (0, 255, 255), 2)
        cv2.putText(
            output,
            "Strongest IR",
            (strongest_point[0] - 40, strongest_point[1] - 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (0, 255, 255),
            1,
        )

    # Debug visualization
    if DEBUG:
        # Create mask visualization
        ir_display = cv2.cvtColor(ir_mask, cv2.COLOR_GRAY2BGR)
        cv2.putText(
            ir_display,
            "IR Mask",
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.7,
            (0, 255, 0),
            2,
        )

        # Information display
        info_display = np.zeros_like(frame)
        detected_count = len(current_frame_points)

        cv2.putText(
            info_display,
            f"Points in current frame: {detected_count}",
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.7,
            (0, 255, 0),
            2,
        )

        cv2.putText(
            info_display,
            f"Current frame: {frame_count}",
            (10, 150),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.7,
            (0, 255, 0),
            2,
        )

        if strongest_point:
            # Find the point data for the strongest point
            strongest_data = next(
                (p for p in current_frame_points if p["position"] == strongest_point),
                None,
            )

            if strongest_data:
                cv2.putText(
                    info_display,
                    f"Strongest IR Angle: {strongest_data['angle']:.1f}°",
                    (10, 70),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.7,
                    (0, 255, 0),
                    2,
                )
                cv2.putText(
                    info_display,
                    f"Strongest IR Distance: {strongest_data['distance']:.1f}",
                    (10, 110),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.7,
                    (0, 255, 0),
                    2,
                )

        # Combine displays
        h, w = frame.shape[:2]
        ir_display = cv2.resize(ir_display, (w // 2, h // 2))
        info_display = cv2.resize(info_display, (w // 2, h // 2))

        debug_top = np.hstack([ir_display, info_display])
        debug_view = np.vstack([debug_top, cv2.resize(output, (w, h // 2))])
        cv2.imshow("IR Detection Debug", debug_view)

    cv2.imshow("IR Detection", output)
    end_time = datetime.datetime.now()
    print(
        f"Frame {frame_count}: Found {len(current_frame_points)} points in {end_time - start_time}"
    )

    # Print points in current frame
    if current_frame_points:
        print(f"Points in frame {frame_count}:")
        for i, point in enumerate(current_frame_points):
            print(
                f"  Point {i+1}: Position {point['position']}, Angle: {point['angle']:.1f}°, Distance: {point['distance']:.1f}"
            )

    if cv2.waitKey(25) & 0xFF == ord("q"):
        break

# Print summary
print(f"Total frames processed: {frame_count}")
print(f"Total points detected across all frames: {all_frames_total_points}")

cap.release()
cv2.destroyAllWindows()

# This makes the current frame's points available
print(
    f"All {len(current_frame_points)} points in the last frame are stored in 'current_frame_points' variable"
)
detected_points = current_frame_points  # Store in a variable that can be used later
