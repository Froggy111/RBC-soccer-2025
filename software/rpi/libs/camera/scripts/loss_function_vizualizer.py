import cv2
import re
import numpy as np

def extract_coordinates(log_text):
    """Extract coordinates from the log text, categorizing as white or non-white."""
    non_white_pixels = []
    white_pixels = []
    
    # Pattern for non-white pixels
    non_white_pattern = r'Non-white pixel at \((\d+), (\d+)\)'
    non_white_matches = re.findall(non_white_pattern, log_text)
    for match in non_white_matches:
        x, y = int(match[0]), int(match[1])
        non_white_pixels.append((x, y))
    
    # Pattern for white pixels
    white_pattern = r'White pixel at \((\d+), (\d+)\)'
    white_matches = re.findall(white_pattern, log_text)
    for match in white_matches:
        x, y = int(match[0]), int(match[1])
        white_pixels.append((x, y))
    
    return non_white_pixels, white_pixels

# Read the log text from the file
with open('loss_function_vizualizer.log', 'r') as file:
    log_content = file.read()

# Extract coordinates from the log
non_white_pixels, white_pixels = extract_coordinates(log_content)
print(f"Found {len(non_white_pixels)} non-white pixels and {len(white_pixels)} white pixels to highlight")

# Load first frame of the video
video_path = '480p.mp4'
cap = cv2.VideoCapture(video_path)
ret, frame = cap.read()
cap.release()

if not ret:
    print("Failed to read the video file")
else:
    # Create a copy of the frame to avoid modifying the original
    highlighted_frame = frame.copy()
    
    # Color non-white pixels in red
    for x, y in non_white_pixels:
        if 0 <= y < highlighted_frame.shape[0] and 0 <= x < highlighted_frame.shape[1]:
            highlighted_frame[y, x] = [0, 0, 255]  # BGR format for red
    
    # Color white pixels in green
    for x, y in white_pixels:
        if 0 <= y < highlighted_frame.shape[0] and 0 <= x < highlighted_frame.shape[1]:
            highlighted_frame[y, x] = [0, 255, 0]  # BGR format for green
    
    # Save the result
    output_path = 'frame.png'
    cv2.imwrite(output_path, highlighted_frame)
    print(f"Highlighted image saved to {output_path}")