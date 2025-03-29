import cv2
import numpy as np

video_path = 'vid.mp4'
cap = cv2.VideoCapture(video_path)

BLUE_LOWER = np.array([100, 150, 50])
BLUE_UPPER = np.array([130, 255, 255])
YELLOW_LOWER = np.array([20, 100, 100])
YELLOW_UPPER = np.array([30, 255, 255])
FIELD_LOWER = np.array([35, 50, 50])  
FIELD_UPPER = np.array([85, 255, 255])

MIN_CONTOUR_AREA = 800
MIN_GOAL_HEIGHT = 120
EDGE_SEARCH_HEIGHT = 50 

def find_true_bottom_edge(frame, bbox, goal_lower, goal_upper):
    """Finds the actual bottom edge by detecting field-goal transition"""
    x, y, w, h = bbox
    
    roi_height = min(h + EDGE_SEARCH_HEIGHT, frame.shape[0] - y)
    roi = frame[y:y+roi_height, x:x+w]  
    
    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    goal_mask = cv2.inRange(hsv, goal_lower, goal_upper)
    field_mask = cv2.inRange(hsv, FIELD_LOWER, FIELD_UPPER)
    
    vertical_proj = np.sum(goal_mask, axis=1)
    
    threshold = 0.2 * np.max(vertical_proj) if len(vertical_proj) > 0 else 0
    goal_rows = np.where(vertical_proj > threshold)[0]
    
    if len(goal_rows) == 0:
        return y + h  
    
    bottom_row = goal_rows[-1]
    for row in range(bottom_row, min(bottom_row + EDGE_SEARCH_HEIGHT, roi_height)):
        if row >= field_mask.shape[0]:  
            break
        if np.sum(field_mask[row,:]) > 0.1 * w * 255:  
            bottom_row = row
            break
    
    return y + bottom_row

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break
    
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    output = frame.copy()
    
    blue_mask = cv2.inRange(hsv, BLUE_LOWER, BLUE_UPPER)
    blue_contours, _ = cv2.findContours(blue_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    if len(blue_contours) > 0:
        largest_blue = max(blue_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_blue) > MIN_CONTOUR_AREA:
            x, y, w, h = cv2.boundingRect(largest_blue)
            if h > MIN_GOAL_HEIGHT:
                # Find true bottom edge
                bottom_y = find_true_bottom_edge(frame, (x,y,w,h), BLUE_LOWER, BLUE_UPPER)
                center_x = x + w//2
                
                # Draw results
                cv2.rectangle(output, (x,y), (x+w,bottom_y), (255,0,0), 2)
                cv2.circle(output, (center_x, bottom_y), 5, (255,255,0), -1)
                cv2.line(output, (center_x-10,bottom_y), (center_x+10,bottom_y), (255,255,0), 2)
                cv2.putText(output, f"({center_x},{bottom_y})", (center_x-40, bottom_y-10), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,0), 1)
    
    yellow_mask = cv2.inRange(hsv, YELLOW_LOWER, YELLOW_UPPER)
    yellow_contours, _ = cv2.findContours(yellow_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    if len(yellow_contours) > 0:
        largest_yellow = max(yellow_contours, key=cv2.contourArea)
        if cv2.contourArea(largest_yellow) > MIN_CONTOUR_AREA:
            x, y, w, h = cv2.boundingRect(largest_yellow)
            if h > MIN_GOAL_HEIGHT:
                bottom_y = find_true_bottom_edge(frame, (x,y,w,h), YELLOW_LOWER, YELLOW_UPPER)
                center_x = x + w//2
                
                cv2.rectangle(output, (x,y), (x+w,bottom_y), (0,255,255), 2)
                cv2.circle(output, (center_x, bottom_y), 5, (0,255,255), -1)
                cv2.line(output, (center_x-10,bottom_y), (center_x+10,bottom_y), (0,255,255), 2)
                cv2.putText(output, f"({center_x},{bottom_y})", (center_x-40, bottom_y-10), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,255), 1)
    
    cv2.imshow('Goal Detection', output)
    if cv2.waitKey(25) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()