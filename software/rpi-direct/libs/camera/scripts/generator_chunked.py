import os
from PIL import Image
import math

def generate_field_coordinates(image_path, output_path, output_image_path, chunk_size=3):
    # Load the image
    img = Image.open(image_path)
    
    # Convert to grayscale if not already
    if img.mode != 'L':
        img = img.convert('L')
    
    # Original dimensions
    original_width, original_height = img.size
    print(f"Original image size: {original_width}x{original_height}")
    
    # Calculate the center of the image in the original scale
    center_x = original_width / 2
    center_y = original_height / 2
    print(f"Center point: ({center_x}, {center_y})")
    
    # Calculate scaling factors:
    x_scale = 320 / 200
    y_scale = 320 / 200
    
    # Calculate number of grid cells in each dimension
    grid_width = math.ceil(original_width / chunk_size)
    grid_height = math.ceil(original_height / chunk_size)
    print(f"Grid dimensions: {grid_width}x{grid_height}")
    
    # Calculate the center of the grid
    grid_center_x = grid_width / 2
    grid_center_y = grid_height / 2
    
    # Process coordinates relative to center
    coordinates = []
    
    # Process the image in chunks
    for grid_y in range(grid_height):
        for grid_x in range(grid_width):
            # Calculate the pixel region this grid cell represents
            start_x = grid_x * chunk_size
            start_y = grid_y * chunk_size
            end_x = min(start_x + chunk_size, original_width)
            end_y = min(start_y + chunk_size, original_height)
            
            # Check if there's any white pixel in this chunk
            has_white = False
            
            # Scan the chunk for white pixels
            for y in range(start_y, end_y):
                for x in range(start_x, end_x):
                    pixel_value = img.getpixel((x, y))
                    if pixel_value == 255:  # White pixel found
                        has_white = True
                        break
                if has_white:
                    break
            
            # If white pixel found in chunk, add the grid coordinate
            if has_white:
                # Convert grid coordinates to be centered at (0,0)
                grid_rel_x = grid_x - grid_center_x
                grid_rel_y = grid_y - grid_center_y
                
                # Convert to field coordinates (in cm)
                # Each grid cell represents chunk_size pixels
                real_x = int(round(grid_rel_x * x_scale))
                real_y = int(round(grid_rel_y * y_scale))
                
                coordinates.append((real_x, real_y))
    
    print(f"Generated {len(coordinates)} coordinates")
    
    # Calculate scaled grid dimensions for the output image
    scaled_grid_width = grid_width * chunk_size * x_scale
    scaled_grid_height = grid_height * chunk_size * y_scale
    
    # Ensure dimensions are integers
    image_width = int(round(scaled_grid_width))
    image_height = int(round(scaled_grid_height))
    
    print(f"Output image size: {image_width}x{image_height}")
    
    # Generate the C++ header file
    with open(output_path, 'w') as f:
        f.write("// Auto-generated field coordinates (320px = 200cm scale, grid-based)\n")
        f.write("#pragma once\n")

        f.write("namespace camera {\n")
        f.write(f"const int WHITE_LINES_LENGTH = {len(coordinates)};\n")
        f.write(f"const int FIELD_WIDTH = {image_width};\n")
        f.write(f"const int FIELD_HEIGHT = {image_height};\n")
        f.write(f"const int GRID_SIZE = {chunk_size};\n\n")
        f.write(f"constexpr int WHITE_LINES[{len(coordinates)}][2] = {{\n")
        
        for i, (x, y) in enumerate(coordinates):
            f.write(f"    {{{x}, {y}}}")
            if i < len(coordinates) - 1:
                f.write(",")
            f.write("\n")
        
        f.write("};\n\n")
        f.write("}")

if __name__ == "__main__":
    # Get the script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Path to the image (update this with your actual image path)
    image_path = os.path.join(script_dir, "field.png")
    
    # Path to output header file
    output_path = os.path.join(script_dir, "../include/field.hpp")
    
    # Path to output image
    output_image_path = os.path.join(script_dir, "field_coordinates.png")
    
    # Ensure output directory exists
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # Generate the coordinates with 3x3 chunks
    generate_field_coordinates(image_path, output_path, output_image_path, chunk_size=3)
    print(f"Field coordinates written to {output_path}")