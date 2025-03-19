import cv2
import numpy as np
import argparse

def unwrap_image(img, center, max_radius, output_size):
    """
    Unwraps a circular mirror image to a rectangular image.
    The vertical axis corresponds to the radial distance, and the horizontal axis to the angle.
    
    Parameters:
      img (ndarray): Input image.
      center (tuple): (x, y) coordinates of the mirror center.
      max_radius (float): Maximum radius (in pixels) from the center to use.
      output_size (tuple): (height, width) of the unwrapped image.
    Returns:
      unwrapped (ndarray): The unwrapped image.
    """
    height, width = output_size

    # Create a grid in polar coordinates:
    # r: from 0 to max_radius, theta: from 0 to 2π.
    r = np.linspace(0, max_radius, height)
    theta = np.linspace(0, 2 * np.pi, width, endpoint=False)
    r_matrix, theta_matrix = np.meshgrid(r, theta, indexing='ij')

    # Convert polar coordinates to Cartesian coordinates.
    map_x = center[0] + r_matrix * np.cos(theta_matrix)
    map_y = center[1] + r_matrix * np.sin(theta_matrix)
    map_x = map_x.astype(np.float32)
    map_y = map_y.astype(np.float32)

    # Remap using OpenCV.
    unwrapped = cv2.remap(img, map_x, map_y, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
    return unwrapped

def vertical_shrink(unwrapped, shrink_coeff):
    """
    Shrinks the unwrapped image vertically. The amount of vertical compression at a given row
    is proportional to the row's distance from the top.
    
    For each original row index i (0 at the top, H-1 at the bottom), we define a scaling factor:
        scale(i) = 1 - shrink_coeff * (i / H)
    Then we compute a new vertical position for each row by accumulating these scales.
    
    Parameters:
      unwrapped (ndarray): The unwrapped image (shape: H x W x channels).
      shrink_coeff (float): Coefficient controlling how much the lower rows are compressed.
                            (Typical values between 0 and 1; 1.0 compresses the bottom the most.)
    Returns:
      shrunk (ndarray): The vertically shrunk image.
    """
    H, W = unwrapped.shape[:2]
    
    # Compute the effective thickness of each row.
    # f[i] represents how much "height" row i contributes.
    f = np.array([1 - shrink_coeff * (i / H) for i in range(H)], dtype=np.float32)
    # Compute cumulative new positions for each original row.
    cum_positions = np.cumsum(f)
    
    # The total new height is given by the last cumulative value.
    new_H = int(np.ceil(cum_positions[-1]))
    
    # Create an array for each new row (from 0 to new_H-1).
    # For each new vertical coordinate, we find the corresponding original row by inverting cum_positions.
    new_rows = np.linspace(0, cum_positions[-1], new_H)
    # Inverse mapping: for each new row, find the corresponding original row index.
    orig_row_indices = np.interp(new_rows, cum_positions, np.arange(H)).astype(np.float32)
    
    # Build a map for vertical remapping.
    # For each pixel in the output, the x-coordinate remains the same.
    map_x = np.tile(np.arange(W, dtype=np.float32), (new_H, 1))
    # For the vertical mapping, each row r in the output corresponds to orig_row_indices[r].
    map_y = np.repeat(orig_row_indices[:, np.newaxis], W, axis=1)
    
    # Use cv2.remap to perform the vertical shrink.
    shrunk = cv2.remap(unwrapped, map_x, map_y, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
    return shrunk

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Unwrap a 360° mirror image and apply vertical shrink based on distance from the top.")
    parser.add_argument('--input', type=str, required=True, help='Path to the input PNG image file.')
    parser.add_argument('--output', type=str, default='output.png', help='Filename for the processed image.')
    parser.add_argument('--center_x', type=float, default=None, help='X coordinate of the mirror center (default: image center).')
    parser.add_argument('--center_y', type=float, default=None, help='Y coordinate of the mirror center (default: image center).')
    parser.add_argument('--max_radius', type=float, default=None, help='Maximum radius (in pixels) from the mirror center (default: min(image width/2, image height/2)).')
    parser.add_argument('--unwrap_height', type=int, default=500, help='Unwrapped image height (number of radial samples).')
    parser.add_argument('--unwrap_width', type=int, default=1000, help='Unwrapped image width (number of angular samples).')
    parser.add_argument('--shrink_coeff', type=float, default=1.0, help='Coefficient for vertical shrink (0=no shrink, 1.0=maximum linear shrink).')
    args = parser.parse_args()

    # Load image.
    img = cv2.imread(args.input, cv2.IMREAD_COLOR)
    if img is None:
        print("Error: Failed to load image. Check the file path.")
        exit(1)

    img_height, img_width = img.shape[:2]
    # Use image center as default.
    center_x = args.center_x if args.center_x is not None else img_width / 2
    center_y = args.center_y if args.center_y is not None else img_height / 2
    center = (center_x, center_y)

    # Default max_radius is the distance from the center to the nearest image edge.
    default_radius = max(center_x, center_y)
    max_radius = args.max_radius if args.max_radius is not None else default_radius

    # Step 1: Unwrap the image.
    unwrapped = unwrap_image(img, center, max_radius, (args.unwrap_height, args.unwrap_width))

    # Step 2: Apply vertical shrink.
    shrunk = vertical_shrink(unwrapped, args.shrink_coeff)

    # Save final output.
    cv2.imwrite(args.output, shrunk)
    print(f"Processed image saved as {args.output}")
