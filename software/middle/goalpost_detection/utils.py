import cv2
import numpy as np

def preprocess_frame(frame, color_lower, color_upper):
    """Preprocess frame to isolate the target color."""
    # Convert to HSV color space
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    
    # Create mask for the specified color range
    mask = cv2.inRange(hsv, color_lower, color_upper)
    
    # Morphological operations to remove noise
    kernel = np.ones((5, 5), np.uint8)
    mask = cv2.erode(mask, kernel, iterations=1)
    mask = cv2.dilate(mask, kernel, iterations=2)
    
    return mask

def find_goal_posts(mask, min_area=500):
    """Find goal posts in the mask."""
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    goal_posts = []
    for contour in contours:
        area = cv2.contourArea(contour)
        if area > min_area:
            x, y, w, h = cv2.boundingRect(contour)
            goal_posts.append((x, y, w, h))
    
    return goal_posts