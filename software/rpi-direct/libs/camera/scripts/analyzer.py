import cv2
import numpy as np
import pandas as pd
import os
import io
import math

data = pd.read_csv("points_data.csv", skipinitialspace=True)

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
    
    # Get the loss value
    loss = row['Loss']
    
    # Draw reference grid
    # Horizontal and vertical lines through center
    cv2.line(frame, (0, center_y), (width, center_y), (50, 50, 50), 1)
    cv2.line(frame, (center_x, 0), (center_x, height), (50, 50, 50), 1)
    
    # Only show points with loss below threshold (using 0.3 as in original)
    # Adjust this threshold as needed for your data
    loss_threshold = 0.4  # Increased to show more points
    
    if loss < loss_threshold:
        # Get coordinates
        x, y = row['X'], row['Y']
        heading = row['Heading']
        
        # Convert to frame coordinates (center is 0,0)
        frame_x = center_x + int(x)
        frame_y = center_y - int(y)  # Flip Y axis to match typical coordinate systems
        
        # Ensure coordinates are within bounds
        frame_x = max(0, min(frame_x, width-1))
        frame_y = max(0, min(frame_y, height-1))
        
        # Draw a red dot
        cv2.circle(frame, (frame_x, frame_y), 5, (0, 0, 255), -1)
        
        # Draw heading line (direction vector)
        line_length = 30
        end_x = frame_x + int(line_length * math.cos(heading))
        end_y = frame_y - int(line_length * math.sin(heading))  # Flip Y for display
        cv2.line(frame, (frame_x, frame_y), (end_x, end_y), (0, 255, 0), 2)
        
        # Add text information
        cv2.putText(frame, f"Frame: {row['Frame']}", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
        cv2.putText(frame, f"X: {x}, Y: {y}", (10, 60), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
        cv2.putText(frame, f"Heading: {heading:.2f}", (10, 90), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
        cv2.putText(frame, f"Loss: {loss:.4f}", (10, 120), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
        cv2.putText(frame, f"Time: {row['Time (ms)']} ms", (10, 150), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
    else:
        # Show frame number and explanation for frames with high loss
        cv2.putText(frame, f"Frame: {row['Frame']} (loss too high: {loss:.4f})", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
    
    # Write frame to video
    out.write(frame)
    
    # Optional: Display progress
    print(f"Processed frame {idx+1}/{len(data)}")

# Release video writer
out.release()
print("Video creation complete!")