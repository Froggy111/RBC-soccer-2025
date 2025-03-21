import os
from PIL import Image

def generate_field_coordinates(image_path, output_path):
    # Load the image
    img = Image.open(image_path)
    
    # Convert to grayscale if not already
    if img.mode != 'L':
        img = img.convert('L')
    
    # Original dimensions and scale
    original_width, original_height = img.size
    print(f"Original image size: {original_width}x{original_height}")
    
    # Calculate the center of the image
    center_x = original_width / 2
    center_y = original_height / 2
    print(f"Center point: ({center_x}, {center_y})")
    
    # Calculate scaling factors:
    # 200cm maps to 320px, so we need to scale our pixel coordinates accordingly
    x_scale = 320 / 200
    y_scale = 320 / 200
    
    # Process coordinates relative to center
    coordinates = []
    
    # Process each pixel in the original image
    for y in range(original_height):
        for x in range(original_width):
            pixel_value = img.getpixel((x, y))
            if pixel_value == 255:  # Assuming white/light pixels represent the field
                # Convert to field coordinates with center as (0,0)
                # Scale directly to match the 320px:200cm ratio
                real_x = int(round((x - center_x) * x_scale))
                real_y = int(round((y - center_y) * y_scale))
                coordinates.append((real_x, real_y))
    
    print(f"Generated {len(coordinates)} coordinates")
    
    # Generate the C++ header file
    with open(output_path, 'w') as f:
        f.write("// Auto-generated field coordinates (320px = 200cm scale)\n")
        f.write("#pragma once\n")
        f.write("#include <tuple>\n\n")
        
        f.write(f"constexpr std::tuple<int, int> COORDINATES[] = {{\n")
        
        for i, (x, y) in enumerate(coordinates):
            f.write(f"    {{{x}, {y}}}")
            if i < len(coordinates) - 1:
                f.write(",")
            f.write("\n")
        
        f.write("};\n\n")

if __name__ == "__main__":
    # Get the script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Path to the image (update this with your actual image path)
    image_path = os.path.join(script_dir, "field.png")
    
    # Path to output header file
    output_path = os.path.join(script_dir, "../include/field.hpp")
    
    # Ensure output directory exists
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # Generate the coordinates
    generate_field_coordinates(image_path, output_path)
    print(f"Field coordinates written to {output_path}")