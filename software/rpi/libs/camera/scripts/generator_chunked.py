import os
import math
from PIL import Image
import sys

def generate_field_coordinates_chunked(
	image_path,
	output_path,
	output_image_path,
	chunk_size=5,
):
	"""
	Generates chunk-based coordinates and a visualization from an image.

	The output visualization image dimensions will be ceil(width/chunk_size) x ceil(height/chunk_size).
	Each pixel (cx, cy) in the output image corresponds to the chunk starting at
	(cx * chunk_size, cy * chunk_size) in the original image. It's colored red
	if any pixel within that original chunk is white.

	The C++ header file will contain the coordinates (cx, cy) of these 'white' chunks.

	Args:
		image_path (str): Path to the input image (expects white lines on black).
		output_path (str): Path to save the generated C++ header file.
		output_image_path (str): Path to save the chunk visualization image.
		chunk_size (int): The size (width and height) of chunks to process.
	"""
	if not os.path.exists(image_path):
		print(f"Error: Input image not found at {image_path}")
		sys.exit(1)
	if chunk_size <= 0:
		print(f"Error: chunk_size must be positive, got {chunk_size}")
		sys.exit(1)

	# Load the image
	try:
		img = Image.open(image_path)
	except Exception as e:
		print(f"Error loading image {image_path}: {e}")
		sys.exit(1)

	# Convert to grayscale if not already
	if img.mode != "L":
		img = img.convert("L")

	original_width, original_height = img.size
	print(f"Original image size: {original_width}x{original_height}")
	print(f"Processing image in {chunk_size}x{chunk_size} chunks...")

	# Calculate the dimensions of the output chunk image
	# Use math.ceil to include partial chunks at the edges
	output_width = math.ceil(original_width / chunk_size)
	output_height = math.ceil(original_height / chunk_size)
	print(f"Output visualization image size: {output_width}x{output_height}")

	# --- Process Chunks and Collect Coordinates ---
	chunk_coordinates = [] # Stores (chunk_x, chunk_y) for white chunks

	for y_chunk_idx in range(output_height):
		for x_chunk_idx in range(output_width):
			# Calculate the pixel range for the current chunk in the original image
			x_chunk_start = x_chunk_idx * chunk_size
			y_chunk_start = y_chunk_idx * chunk_size
			# Use min() to ensure we don't go past the original image boundaries
			x_chunk_end = min(x_chunk_start + chunk_size, original_width)
			y_chunk_end = min(y_chunk_start + chunk_size, original_height)

			chunk_is_white = False
			# Check pixels within the current chunk
			for y in range(y_chunk_start, y_chunk_end):
				for x in range(x_chunk_start, x_chunk_end):
					try:
						pixel_value = img.getpixel((x, y))
						# Use a threshold slightly below 255 for robustness if needed
						# For pure white on black, 255 is fine.
						if pixel_value >= 250: # Threshold for 'white'
							chunk_is_white = True
							break # Found white, no need to check more in this row
					except IndexError:
						# Should not happen with correct loop bounds, but safety first
						print(f"Warning: Pixel access out of bounds at ({x},{y})")
						continue
				if chunk_is_white:
					break # Found white, no need to check more rows in this chunk

			# If the chunk contained a white pixel, store its indices
			if chunk_is_white:
				chunk_coordinates.append((x_chunk_idx, y_chunk_idx))

	if not chunk_coordinates:
		print("Warning: No white pixels found in any chunk. Output will reflect this.")
	else:
		print(f"Found {len(chunk_coordinates)} chunks containing white pixels.")


	# --- Create and Save the Chunk Visualization Image ---
	output_img = Image.new("RGB", (output_width, output_height), "black")
	pixels = output_img.load()

	for cx, cy in chunk_coordinates:
		if 0 <= cx < output_width and 0 <= cy < output_height:
			pixels[cx, cy] = (255, 0, 0)  # Red for chunks containing white
		else:
			# Should not happen if logic is correct
			print(f"Warning: Chunk index ({cx},{cy}) out of bounds for output image size ({output_width}x{output_height}).")

	try:
		output_img.save(output_image_path)
		print(f"Chunk visualization image written to {output_image_path}")
	except Exception as e:
		print(f"Error saving visualization image {output_image_path}: {e}")


	# --- Generate the C++ header file with Chunk Coordinates ---
	try:
		with open(output_path, "w") as f:
			f.write(f"// Auto-generated field chunk coordinates\n")
			f.write(f"// Generated from: {os.path.basename(image_path)}\n")
			f.write(f"// Chunk Size: {chunk_size}x{chunk_size} pixels\n")
			f.write(f"// Coordinates represent the indices (cx, cy) of chunks containing white.\n")
			f.write("#pragma once\n\n")

			f.write("namespace field_model {\n\n")
			f.write(f"constexpr int CHUNK_SIZE = {chunk_size};\n")
			f.write(f"constexpr int FIELD_CHUNKS_WIDTH = {output_width};\n")
			f.write(f"constexpr int FIELD_CHUNKS_HEIGHT = {output_height};\n")
			f.write(f"constexpr int WHITE_CHUNK_COUNT = {len(chunk_coordinates)};\n\n")

			if chunk_coordinates:
				f.write(f"constexpr int WHITE_CHUNK_INDICES[WHITE_CHUNK_COUNT][2] = {{\n")
				for i, (cx, cy) in enumerate(chunk_coordinates):
					f.write(f"    {{{cx}, {cy}}}")
					if i < len(chunk_coordinates) - 1:
						f.write(",")
					f.write("\n")
				f.write("};\n")
			else:
				# Handle empty case gracefully in C++ if needed
				f.write("// No white chunks found.\n")
				f.write("constexpr int WHITE_CHUNK_INDICES[1][2] = {{ {0, 0} }}; // Placeholder or handle empty array\n")

			f.write("\n} // namespace field_model\n")
		print(f"C++ header file with chunk indices written to {output_path}")

	except Exception as e:
		print(f"Error writing C++ header file {output_path}: {e}")


if __name__ == "__main__":
	try:
		script_dir = os.path.dirname(os.path.abspath(__file__))
	except NameError:
		script_dir = os.getcwd()
		print(f"Warning: Could not determine script directory via __file__, using current working directory: {script_dir}")

	# --- Configuration ---
	image_name = "field.png"
	# Relative path needs careful handling, ensure '../include' exists relative to script_dir
	output_header_relative_path = "../include/field_model_chunks.hpp"
	output_image_name = "field_chunks_visualization.png"
	chunk_processing_size = 10  # Pixels per chunk (e.g., 10x10)
	# --- End Configuration ---

	# Construct full paths
	image_path = os.path.join(script_dir, image_name)
	output_path = os.path.normpath(os.path.join(script_dir, output_header_relative_path))
	output_image_path = os.path.join(script_dir, output_image_name)

	# Ensure output directory for header exists
	output_dir = os.path.dirname(output_path)
	if not os.path.exists(output_dir):
		print(f"Creating output directory: {output_dir}")
		os.makedirs(output_dir, exist_ok=True)

	# Generate the chunk-based coordinates and visualization
	generate_field_coordinates_chunked(
		image_path=image_path,
		output_path=output_path,
		output_image_path=output_image_path,
		chunk_size=chunk_processing_size,
	)

	print("\nScript finished.")