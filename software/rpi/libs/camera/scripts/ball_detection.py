import cv2
import numpy as np
import os
import datetime
import math
import time
from picamera2 import Picamera2
from flask import Flask, Response, render_template
import threading

DEBUG = True
SAVE_POINTS = False

# Constants for IR detection
MIN_CONTOUR_AREA = 10
MIN_BRIGHTNESS = 130
CENTRE_POINT = [640 // 2 - 5, 480 // 2 + 30]

# Adjusted Purple HSV range
PURPLE_LOWER = np.array([130, 50, 100])
PURPLE_UPPER = np.array([175, 255, 255])

# Global variables for frame sharing between threads
global_frame = None
global_output = None
global_debug_view = None
lock = threading.Lock()
current_frame_points = []

app = Flask(__name__)

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

def process_frame(frame):
    """Process a single frame for IR detection."""
    global current_frame_points
    
    output = frame.copy()

    # Mark center point
    cv2.circle(output, tuple(CENTRE_POINT), 5, (255, 255, 255), -1)

    # Detect IR points - this returns all purple points in the current frame
    current_frame_points, ir_mask = detect_ir_points(frame)

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

    debug_view = None
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

    return output, debug_view

def camera_thread_function():
    global global_frame, global_output, global_debug_view
    
    # Initialize PiCamera
    picam2 = Picamera2()
    config = picam2.create_preview_configuration(main={"size": (640, 480)})
    picam2.configure(config)
    picam2.start()
    
    frame_count = 0
    all_frames_total_points = 0
    
    try:
        while True:
            # Capture frame from camera
            frame = picam2.capture_array()
            frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)  # Convert from RGB to BGR
            
            frame_count += 1
            start_time = datetime.datetime.now()
            
            # Process the frame
            output, debug_view = process_frame(frame)
            
            # Update statistics
            all_frames_total_points += len(current_frame_points)
            
            # Update global frames with thread safety
            with lock:
                global_frame = frame.copy()
                global_output = output.copy()
                if debug_view is not None:
                    global_debug_view = debug_view.copy()
            
            end_time = datetime.datetime.now()
            print(f"Frame {frame_count}: Found {len(current_frame_points)} points in {end_time - start_time}")
            
            # Sleep for a moment to avoid excessive CPU usage
            time.sleep(0.01)
    
    finally:
        picam2.stop()

def generate_frames(source):
    global global_frame, global_output, global_debug_view
    
    while True:
        # Select the appropriate frame based on the source
        with lock:
            if source == 'raw' and global_frame is not None:
                frame_to_send = global_frame.copy()
            elif source == 'processed' and global_output is not None:
                frame_to_send = global_output.copy()
            elif source == 'debug' and global_debug_view is not None:
                frame_to_send = global_debug_view.copy()
            else:
                continue
        
        # Convert to JPEG
        ret, buffer = cv2.imencode('.jpg', frame_to_send)
        if not ret:
            continue
            
        # Yield the output frame in the byte format
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + buffer.tobytes() + b'\r\n')

@app.route('/')
def index():
    return '''
    <!DOCTYPE html>
    <html>
    <head>
        <title>Ball Detection Stream</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 0;
                padding: 20px;
                background-color: #f0f0f0;
                text-align: center;
            }
            h1 {
                color: #333;
            }
            .stream-container {
                display: flex;
                flex-direction: column;
                align-items: center;
                gap: 20px;
                margin-top: 20px;
            }
            .stream-box {
                border: 2px solid #333;
                border-radius: 5px;
                overflow: hidden;
            }
            .stream-box h2 {
                background-color: #333;
                color: white;
                margin: 0;
                padding: 10px;
            }
            img {
                max-width: 100%;
            }
        </style>
    </head>
    <body>
        <h1>Ball Detection Stream</h1>
        <div class="stream-container">
            <div class="stream-box">
                <h2>Processed Stream</h2>
                <img src="/video_feed/processed" alt="Processed Stream">
            </div>
            <div class="stream-box">
                <h2>Debug View</h2>
                <img src="/video_feed/debug" alt="Debug View">
            </div>
            <div class="stream-box">
                <h2>Raw Camera Feed</h2>
                <img src="/video_feed/raw" alt="Raw Camera Feed">
            </div>
        </div>
    </body>
    </html>
    '''

@app.route('/video_feed/<source>')
def video_feed(source):
    """Video streaming route."""
    return Response(generate_frames(source),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    # Start camera thread
    camera_thread = threading.Thread(target=camera_thread_function)
    camera_thread.daemon = True
    camera_thread.start()
    
    # Start the Flask server
    print("Starting web server, access the stream at http://localhost:5000/")
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)