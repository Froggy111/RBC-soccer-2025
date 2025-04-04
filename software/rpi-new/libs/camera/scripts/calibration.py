import cv2
import numpy as np
import os

image_path = "RBC-soccer-2025/software/rpi-direct/libs/camera/scripts/blue6.py"


def get_color_range(image_path):
    # Load reference image
    img = cv2.imread(image_path)
    if img is None:
        print(f"Error: Could not load {image_path}")
        print(f"Current directory: {os.getcwd()}")
        print(f"Files in directory: {os.listdir()}")
        return None

    # Rest of your function remains the same...
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    rect = cv2.selectROI("Select Goal Post Area", img)
    cv2.destroyAllWindows()
    x, y, w, h = rect
    roi = hsv[y : y + h, x : x + w]
    h_min, s_min, v_min = roi.min(axis=(0, 1))
    h_max, s_max, v_max = roi.max(axis=(0, 1))
    h_tolerance = 10
    s_tolerance = 40
    v_tolerance = 40
    lower = np.array([h_min - h_tolerance, s_min - s_tolerance, v_min - v_tolerance])
    upper = np.array([h_max + h_tolerance, s_max + s_tolerance, v_max + v_tolerance])
    lower = np.clip(lower, 0, 179)
    upper = np.clip(upper, 0, 255)
    return lower, upper


# Get script directory and construct image path
script_dir = os.path.dirname(os.path.abspath(__file__))
ref_image = os.path.join(script_dir, "blue6.png")

# Verify path
print(f"Looking for image at: {ref_image}")
if not os.path.exists(ref_image):
    print("Error: Image file does not exist at specified path")
    exit()

lower_blue, upper_blue = get_color_range(ref_image)

if lower_blue is not None:
    print(f"Blue Goal HSV Range:")
    print(f"Lower: {lower_blue}")
    print(f"Upper: {upper_blue}")
