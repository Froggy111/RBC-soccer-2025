import cv2
import numpy as np
import os

def process_first_frame(video_path, threshold=200):
    # Check if video file exists
    if not os.path.exists(video_path):
        print(f"Error: Video file '{video_path}' not found")
        return

    # Open the video file
    cap = cv2.VideoCapture(video_path)
    
    # Check if video opened successfully
    if not cap.isOpened():
        print(f"Error: Could not open video file '{video_path}'")
        return
    
    # Read the first frame
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read the first frame")
        return
    
    # Release the video capture object
    cap.release()
    
    # Save the original frame
    cv2.imwrite('original_frame.jpg', frame)
    print(f"Original frame saved as 'original_frame.jpg'")
    
    # Create a binary mask where pixels with R, G, and B all above threshold are white
    mask = np.all(frame >= threshold, axis=2).astype(np.uint8) * 255
    
    # Create a filtered image (black background with white pixels where condition is met)
    filtered_image = np.zeros_like(frame)
    filtered_image[mask == 255] = [255, 255, 255]
    
    # Save the filtered image
    cv2.imwrite('filtered_white_pixels.jpg', filtered_image)
    print(f"Filtered image saved as 'filtered_white_pixels.jpg'")
    
    # Create a visualization with white pixels highlighted on the original image
    highlighted = frame.copy()
    highlighted[mask == 255] = [0, 255, 255]  # Highlight in yellow for visibility
    
    # Save the highlighted image
    cv2.imwrite('highlighted_white_pixels.jpg', highlighted)
    print(f"Highlighted image saved as 'highlighted_white_pixels.jpg'")

if __name__ == "__main__":
    video_path = "RBC-soccer-2025/software/rpi-direct/libs/camera/scripts/vid2.mp4"  # Update this path if your video is in a different location
    process_first_frame(video_path, threshold=160)
    print("Processing complete!")