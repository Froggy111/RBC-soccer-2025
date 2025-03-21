import cv2
import numpy as np
import argparse

def unwrap_image(img, center, max_radius, output_size):
    """
    Unwraps a circular mirror image to a rectangular image.

    Parameters:
      img (ndarray): Input image.
      center (tuple): (x, y) coordinates of the mirror center in the image.
      max_radius (float): Maximum radius (in pixels) from the center to be used.
      output_size (tuple): Desired output size (height, width) where
                           height corresponds to radial distance and width to the angle.
    Returns:
      unwrapped (ndarray): The unwrapped (rectangular) image.
    """
    height, width = output_size

    # Create a grid for polar coordinates:
    # - r varies from 0 to max_radius (vertical axis)
    # - θ varies from 0 to 2π (horizontal axis)
    r = np.linspace(0, max_radius, height)
    theta = np.linspace(0, 2 * np.pi, width, endpoint=False)
    r_matrix, theta_matrix = np.meshgrid(r, theta, indexing='ij')  # shape: (height, width)

    # Convert polar coordinates to Cartesian coordinates:
    map_x = center[0] + r_matrix * np.cos(theta_matrix)
    map_y = center[1] + r_matrix * np.sin(theta_matrix)
    map_x = map_x.astype(np.float32)
    map_y = map_y.astype(np.float32)

    # Remap the input image to the unwrapped output
    unwrapped = cv2.remap(img, map_x, map_y, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
    return unwrapped

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Unwrap a 360° mirror image into a rectangular image.")
    parser.add_argument('--input', type=str, required=True, help='Path to the input PNG image file.')
    parser.add_argument('--output', type=str, default='output.png', help='Filename for the unwrapped image.')
    parser.add_argument('--center_x', type=float, default=None, help='X coordinate of the mirror center (default: image center).')
    parser.add_argument('--center_y', type=float, default=None, help='Y coordinate of the mirror center (default: image center).')
    parser.add_argument('--max_radius', type=float, default=None, help='Maximum radius (in pixels) from the mirror center (default: min(image width/2, image height/2)).')
    parser.add_argument('--height', type=int, default=500, help='Output image height (number of radial samples).')
    parser.add_argument('--width', type=int, default=1000, help='Output image width (number of angular samples).')
    args = parser.parse_args()

    # Load the image in color mode
    img = cv2.imread(args.input, cv2.IMREAD_COLOR)
    if img is None:
        print("Error: Failed to load image. Check the file path.")
        exit(1)

    img_height, img_width = img.shape[:2]
    
    # Default center is the image center if not provided
    center_x = args.center_x if args.center_x is not None else img_width / 2
    center_y = args.center_y if args.center_y is not None else img_height / 2
    center = (center_x, center_y)

    # Default max_radius is the distance from the center to the nearest image edge
    default_radius = max(center_x, center_y)
    max_radius = args.max_radius if args.max_radius is not None else default_radius

    output_size = (args.height, args.width)

    # Generate the unwrapped image
    unwrapped = unwrap_image(img, center, max_radius, output_size)

    # Save the unwrapped image
    cv2.imwrite(args.output, unwrapped)
    print(f"Unwrapped image saved as {args.output}")
