import cv2
import numpy as np
import pandas as pd
import os

# Parameters
OUTPUT_VIDEO = "visualization.mp4"
VIDEO_WIDTH = 291
VIDEO_HEIGHT = 350
RED_DOT_RADIUS = 3
FPS = 30
BACKGROUND_COLOR = (255, 255, 255)  # White
DOT_COLOR = (0, 0, 255)  # Red in BGR

# Read the CSV file
print("Reading data file...")
df = pd.read_csv('points_data.csv', skiprows=0)

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
    
    # Ensure coordinates are within bounds
    x = max(0, min(x, VIDEO_WIDTH-1))
    y = max(0, min(y, VIDEO_HEIGHT-1))
    
    # Create blank frame
    frame = np.ones((VIDEO_HEIGHT, VIDEO_WIDTH, 3), dtype=np.uint8) * 255
    
    # Draw red dot at coordinates
    cv2.circle(frame, (x, y), RED_DOT_RADIUS, DOT_COLOR, -1)
    
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