import cv2
import numpy as np
import pandas as pd
import os

# Read the CSV file
data = pd.read_csv('points_data.csv')

# Set up video parameters
width, height = 853, 480
fps = 30
fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter('visualization.mp4', fourcc, fps, (width, height))

# Center point coordinates
center_x, center_y = width // 2, height // 2

# Process each frame
for idx, row in data.iterrows():
    # Create a blank black frame
    frame = np.zeros((height, width, 3), dtype=np.uint8)
    
    # Find the point with the lowest loss
    loss_values = [row['tl_loss'], row['tr_loss'], row['bl_loss'], row['br_loss']]
    min_loss_idx = np.argmin(loss_values)
    min_loss = loss_values[min_loss_idx]
    
    # Only show points with loss below 0.3
    if min_loss < 0.3:
        # Get coordinates based on the lowest loss
        if min_loss_idx == 0:  # top-left
            x, y = row['tl_x'], row['tl_y']
        elif min_loss_idx == 1:  # top-right
            x, y = row['tr_x'], row['tr_y']
        elif min_loss_idx == 2:  # bottom-left
            x, y = row['bl_x'], row['bl_y']
        else:  # bottom-right
            x, y = row['br_x'], row['br_y']
        
        # Convert to frame coordinates (center is 0,0)
        frame_x = center_x + int(x)
        frame_y = center_y + int(y)
        
        # Ensure coordinates are within bounds
        frame_x = max(0, min(frame_x, width-1))
        frame_y = max(0, min(frame_y, height-1))
        
        # Draw a red dot (slightly bigger for visibility)
        cv2.circle(frame, (frame_x, frame_y), 3, (0, 0, 255), -1)
        
        # Add loss value text (optional)
        cv2.putText(frame, f"Loss: {min_loss:.3f}", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
    
    # Write frame to video (all frames are included, but only good ones have dots)
    out.write(frame)
    
    # Optional: Display progress
    if idx % 100 == 0:
        print(f"Processed frame {idx}/{len(data)}")

# Release video writer
out.release()
print("Video creation complete!")