import cv2
import numpy as np
import pandas as pd
import math

# Parameters
OUTPUT_VIDEO = "visualization.mp4"
VIDEO_WIDTH = 253
VIDEO_HEIGHT = 312
RED_DOT_RADIUS = 3
ARROW_LENGTH = 15  # Length of the heading arrow
FPS = 30
BACKGROUND_COLOR = (255, 255, 255)  # White
DOT_COLOR = (0, 0, 255)  # Red in BGR
ARROW_COLOR = (0, 0, 255)  # Red in BGR

# Read the CSV file
print("Reading data file...")
df = pd.read_csv('points_data.csv', skiprows=0)

# Load background image
print("Loading background image...")
background = None
try:
    background = cv2.imread('field.png')
    # Resize if needed
    if background is not None and (background.shape[1] != VIDEO_WIDTH or background.shape[0] != VIDEO_HEIGHT):
        background = cv2.resize(background, (VIDEO_WIDTH, VIDEO_HEIGHT))
    print("Background image loaded successfully")
except Exception as e:
    print(f"Could not load background image: {e}")
    print("Using white background instead")
    background = np.ones((VIDEO_HEIGHT, VIDEO_WIDTH, 3), dtype=np.uint8) * 255

# Create video writer
print(f"Creating video with dimensions {VIDEO_WIDTH}x{VIDEO_HEIGHT}")
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(OUTPUT_VIDEO, fourcc, FPS, (VIDEO_WIDTH, VIDEO_HEIGHT))

# Create frames and add to video
print("Generating frames...")
total_frames = len(df)

for idx, row in df.iterrows():
    # Get coordinates
    x = int(row[1])
    y = int(row[2])
    heading = int(row[3]) # in degrees, anticlockwise from north
    
    # Ensure coordinates are within bounds
    x = VIDEO_WIDTH // 2 - x
    y = VIDEO_HEIGHT // 2 + y
    
    # Create frame from background
    frame = background.copy()
    
    # Draw red dot at coordinates
    cv2.circle(frame, (x, y), RED_DOT_RADIUS, DOT_COLOR, -1)
    
    # Calculate arrow endpoint using heading
    # Convert heading to radians and adjust for OpenCV coordinate system
    # In OpenCV, 0 degrees is to the right and increases clockwise
    # Convert from anticlockwise from north to clockwise from east
    heading_rad = math.radians((90 - heading) % 360)
    
    # Calculate endpoint of arrow
    end_x = int(x + ARROW_LENGTH * math.cos(heading_rad))
    end_y = int(y - ARROW_LENGTH * math.sin(heading_rad))
    
    # Draw arrow for heading direction
    cv2.arrowedLine(frame, (x, y), (end_x, end_y), ARROW_COLOR, 2, tipLength=0.4)
    
    # Add frame number
    cv2.putText(frame, f"Frame: {idx}", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
    
    # Add to video
    out.write(frame)
    
    # Show progress
    if idx % 100 == 0:
        print(f"Processed {idx}/{total_frames} frames ({idx/total_frames*100:.1f}%)")

# Release resources
out.release()
print(f"Video created successfully: {OUTPUT_VIDEO}")