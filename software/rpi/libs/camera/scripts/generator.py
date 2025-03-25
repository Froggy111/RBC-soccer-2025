import os
from PIL import Image


def generate_field_coordinates(image_path, output_path, output_image_path):
    # Load the image
    img = Image.open(image_path)

    # Convert to grayscale if not already
    if img.mode != "L":
        img = img.convert("L")

    # Original dimensions and scale
    original_width, original_height = img.size
    print(f"Original image size: {original_width}x{original_height}")

    # Calculate the center of the image
    center_x = original_width / 2
    center_y = original_height / 2
    print(f"Center point: ({center_x}, {center_y})")

    # Calculate scaling factors:
    x_scale = 320 / 200
    y_scale = 320 / 200

    # Process coordinates relative to center
    coordinates = []
    count = 0

    # Process each pixel in the original image
    for y in range(original_height):
        for x in range(original_width):
            pixel_value = img.getpixel((x, y))
            if pixel_value == 255:  # Assuming white/light pixels represent the field
                # Convert to field coordinates with center as (0,0)
                real_x = int(round((x - center_x) * x_scale))
                real_y = int(round((y - center_y) * y_scale))
                if count % 2 == 0:
                    coordinates.append((real_x, real_y))
                count += 1

    print(f"Generated {len(coordinates)} coordinates")

    # Calculate image dimensions directly from scaling factors
    image_width = int(round(original_width * x_scale))
    image_height = int(round(original_height * y_scale))

    print(f"Output image size: {image_width}x{image_height}")

    # Create a black image
    output_img = Image.new("RGB", (image_width, image_height), "black")
    pixels = output_img.load()

    # Set pixels to red based on coordinates
    for x, y in coordinates:
        # Shift coordinates to be within image bounds
        pixel_x = x + int(image_width / 2)
        pixel_y = y + int(image_height / 2)

        # Check if the pixel is within the image bounds
        if 0 <= pixel_x < image_width and 0 <= pixel_y < image_height:
            pixels[pixel_x, pixel_y] = (255, 0, 0)  # Red

    # Save the image
    output_img.save(output_image_path)
    print(f"Coordinate image written to {output_image_path}")

    # Generate the C++ header file
    with open(output_path, "w") as f:
        f.write("// Auto-generated field coordinates (320px = 200cm scale)\n")
        f.write("#pragma once\n")

        f.write("namespace camera {\n")
        f.write(f"const int WHITE_LINES_LENGTH = {len(coordinates)};\n")
        f.write(f"const int FIELD_X_SIZE = {image_width};\n")
        f.write(f"const int FIELD_Y_SIZE = {image_height};\n\n")
        f.write("const int GRID_SIZE = 1;\n")
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

    # Generate the coordinates
    generate_field_coordinates(image_path, output_path, output_image_path)
    print(f"Field coordinates written to {output_path}")
