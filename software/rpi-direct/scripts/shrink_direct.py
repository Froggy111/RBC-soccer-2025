import numpy as np
from PIL import Image

def center_shrink(image_path, output_path, shrink_factor=0.3, output_width=None, output_height=None):
    """
    Apply a center-based shrink/expand effect to an image and place it in a larger canvas.
    
    Args:
        image_path (str): Path to the input image
        output_path (str): Path to save the output image
        shrink_factor (float): Intensity of the effect
            Positive values (0.0 to 1.0): Shrink the center
            Negative values: Expand the center
        output_width (int, optional): Width of the output image. If None, uses input width * 2.
        output_height (int, optional): Height of the output image. If None, uses input height * 2.
    """
    # Load image and convert to numpy array
    img = Image.open(image_path)
    input_width, input_height = img.size
    img_array = np.array(img)
    
    # Set output dimensions
    output_width = output_width or input_width * 3
    output_height = output_height or input_height * 3
    
    # Create coordinate grids for the output image
    y_dest, x_dest = np.mgrid[0:output_height, 0:output_width]
    
    # Calculate center coordinates of the output image
    center_y, center_x = output_height // 2, output_width // 2
    
    # Calculate distance from center (normalized to [0, 1])
    max_distance = np.sqrt(max(center_x, center_y)**2 + max(center_x, center_y)**2)
    distance = np.sqrt((x_dest - center_x)**2 + (y_dest - center_y)**2) / max_distance
    
    # Calculate the shrink factor based on distance
    # Pixels near center (distance=0) get maximum effect
    # Pixels at edge (distance=1) get no effect
    shrink_map = 1.0 - shrink_factor * (1.0 - distance)
    
    # Prevent division by zero by setting a minimum value
    # This handles the case where shrink_factor = 1.0
    shrink_map = np.maximum(shrink_map, 0.001)
    
    # Calculate source coordinates in the original image space
    # We're mapping from destination to source (inverse mapping)
    source_x = center_x + (x_dest - center_x) / shrink_map
    source_y = center_y + (y_dest - center_y) / shrink_map
    
    # Convert source coordinates to input image space
    source_x_input = source_x - (output_width - input_width) // 2
    source_y_input = source_y - (output_height - input_height) // 2
    
    # Create a mask for valid coordinates (inside the original image)
    valid_coords = (
        (source_x_input >= 0) & 
        (source_x_input < input_width - 1) &  # Subtract 1 to avoid edge cases
        (source_y_input >= 0) & 
        (source_y_input < input_height - 1)   # Subtract 1 to avoid edge cases
    )
    
    # For valid pixels, round to nearest integer for sampling
    source_x_valid = np.round(source_x_input[valid_coords]).astype(int)
    source_y_valid = np.round(source_y_input[valid_coords]).astype(int)
    
    # Create output image with proper dimensions
    if len(img_array.shape) == 3:  # RGB/RGBA image
        channels = img_array.shape[2]
        
        # Create transparent background if RGBA image
        if channels == 4:
            output = np.zeros((output_height, output_width, channels), dtype=img_array.dtype)
            output[:, :, 3] = 0  # Set alpha channel to fully transparent
        else:
            output = np.zeros((output_height, output_width, channels), dtype=img_array.dtype)
        
        # Map pixels from source to destination only for valid coordinates
        for c in range(channels):
            output[y_dest[valid_coords], x_dest[valid_coords], c] = img_array[source_y_valid, source_x_valid, c]
    else:  # Grayscale image
        output = np.zeros((output_height, output_width), dtype=img_array.dtype)
        output[y_dest[valid_coords], x_dest[valid_coords]] = img_array[source_y_valid, source_x_valid]
    
    # Convert back to PIL and save
    output_img = Image.fromarray(output)
    
    # If original image was RGBA but our array doesn't have alpha (PIL conversion issue),
    # convert to RGBA mode explicitly
    if img.mode == 'RGBA' and output_img.mode != 'RGBA':
        output_img = output_img.convert('RGBA')
    
    output_img.save(output_path)
    print(f"Processed image saved to {output_path}")

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Apply center-based shrink/expand effect to an image")
    parser.add_argument("input_image", help="Path to the input image")
    parser.add_argument("output_image", help="Path to save the output image")
    parser.add_argument("-f", "--factor", type=float, default=-3, 
                        help="Effect factor (positive: shrink, negative: expand), default=0.3")
    parser.add_argument("-wi", "--width", type=int, help="Output image width")
    parser.add_argument("-hi", "--height", type=int, help="Output image height")
    
    args = parser.parse_args()
    
    center_shrink(args.input_image, args.output_image, args.factor, args.width, args.height)