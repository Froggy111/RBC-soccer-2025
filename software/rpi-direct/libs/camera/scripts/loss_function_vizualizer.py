import numpy as np
import matplotlib.pyplot as plt
import argparse
import re
import os

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Visualize transformed field coordinates')
    parser.add_argument('--guess_x', type=float, default=0, help='X coordinate of guess position')
    parser.add_argument('--guess_y', type=float, default=0, help='Y coordinate of guess position')
    parser.add_argument('--guess_heading', type=float, default=0, help='Heading angle in radians')
    args = parser.parse_args()
    
    # Constants from field.hpp
    field_data = read_field_hpp()
    WHITE_LINES_LENGTH = field_data['WHITE_LINES_LENGTH']
    FIELD_WIDTH = field_data['FIELD_WIDTH']
    FIELD_HEIGHT = field_data['FIELD_HEIGHT']
    GRID_SIZE = field_data['GRID_SIZE']
    white_lines = field_data['WHITE_LINES']
    
    # Fixed-point scaling factor (Q16.16 format)
    FP_SHIFT = 16
    FP_ONE = 1 << FP_SHIFT
    
    # Make sure headings are in bound
    args.guess_heading = abs(args.guess_heading) % (2 * np.pi)

    # Convert angles to fixed point representation
    sin_theta_fp = int(np.sin(args.guess_heading) * FP_ONE)
    cos_theta_fp = int(np.cos(args.guess_heading) * FP_ONE)
    
    # Arrays to store transformed coordinates
    transformed_x = []
    transformed_y = []
    
    # Transform & rotate white line coords
    for i in range(WHITE_LINES_LENGTH):
        x = white_lines[i][0]
        y = white_lines[i][1]
        
        # Transform to relative coordinates
        rel_x = int(x + args.guess_x / GRID_SIZE)
        rel_y = int(y + args.guess_y / GRID_SIZE)
        
        # Rotate using fixed-point arithmetic (ensure integer operations)
        rotated_x_fp = (rel_x * cos_theta_fp - rel_y * sin_theta_fp) >> FP_SHIFT
        rotated_y_fp = (rel_x * sin_theta_fp + rel_y * cos_theta_fp) >> FP_SHIFT
        
        # Convert back to integer coordinate space
        final_x = int(rotated_x_fp + 480 / 2)
        final_y = int(rotated_y_fp + 640 / 2)
        
        # Check if the point is within image boundaries
        if final_x < 0 or final_x >= 480 or final_y < 0 or final_y >= 640:
            continue
        
        transformed_x.append(final_x)
        transformed_y.append(final_y)

    # Create visualization
    plt.figure(figsize=(10, 8))
    plt.scatter(transformed_x, transformed_y, s=2, c='blue', alpha=0.7)
    
    # Configure plot
    plt.xlim(0, 480)
    plt.ylim(0, 640)
    plt.grid(True, alpha=0.3)
    plt.title(f'Transformed Field Coordinates\nGuess: ({args.guess_x}, {args.guess_y}, {args.guess_heading:.2f} rad)')
    plt.xlabel('X Coordinate')
    plt.ylabel('Y Coordinate')

    # Draw center of the field
    plt.scatter(480 / 2, 640 / 2, s=100, c='red', label='Field Center')

    plt.show()
    
    print(f"Transformed {len(transformed_x)} points out of {WHITE_LINES_LENGTH}")

def read_field_hpp():
    """Read field parameters and coordinates from field.hpp file"""
    # Path to the field.hpp file relative to this script
    field_hpp_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 
                                  "..", "include", "field.hpp")
    
    if not os.path.exists(field_hpp_path):
        raise FileNotFoundError(f"Could not find field.hpp at {field_hpp_path}")
    
    with open(field_hpp_path, 'r') as f:
        content = f.read()
    
    # Extract parameters
    params = {}
    
    # Extract WHITE_LINES_LENGTH
    match = re.search(r'const\s+int\s+WHITE_LINES_LENGTH\s*=\s*(\d+)', content)
    if match:
        params['WHITE_LINES_LENGTH'] = int(match.group(1))
    else:
        raise ValueError("Could not find WHITE_LINES_LENGTH in field.hpp")
    
    # Extract FIELD_WIDTH
    match = re.search(r'const\s+int\s+FIELD_WIDTH\s*=\s*(\d+)', content)
    if match:
        params['FIELD_WIDTH'] = int(match.group(1))
    else:
        raise ValueError("Could not find FIELD_WIDTH in field.hpp")
    
    # Extract FIELD_HEIGHT
    match = re.search(r'const\s+int\s+FIELD_HEIGHT\s*=\s*(\d+)', content)
    if match:
        params['FIELD_HEIGHT'] = int(match.group(1))
    else:
        raise ValueError("Could not find FIELD_HEIGHT in field.hpp")
    
    # Extract GRID_SIZE
    match = re.search(r'const\s+int\s+GRID_SIZE\s*=\s*(\d+)', content)
    if match:
        params['GRID_SIZE'] = int(match.group(1))
    else:
        params['GRID_SIZE'] = 1  # Default value if not found
    
    # Extract WHITE_LINES coordinates
    # Find the array declaration
    match = re.search(r'constexpr\s+int\s+WHITE_LINES\[\d+\]\[\d+\]\s*=\s*{(.*?)};', 
                      content, re.DOTALL)
    
    if not match:
        raise ValueError("Could not find WHITE_LINES array in field.hpp")
    
    array_content = match.group(1)
    
    # Parse all coordinate pairs
    coords = []
    coord_pattern = re.compile(r'{([^}]+)}')
    for coord_match in coord_pattern.finditer(array_content):
        coord_str = coord_match.group(1)
        x, y = map(int, coord_str.split(','))
        coords.append([x, y])
    
    params['WHITE_LINES'] = coords
    
    # Verify that we parsed the correct number of coordinates
    if len(coords) != params['WHITE_LINES_LENGTH']:
        print(f"Warning: Parsed {len(coords)} coordinates but WHITE_LINES_LENGTH is {params['WHITE_LINES_LENGTH']}")
    
    return params

if __name__ == "__main__":
    main()