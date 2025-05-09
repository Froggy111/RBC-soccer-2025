import numpy as np
import math
import argparse
import cv2 # Using OpenCV to save the image, Pillow (PIL) could also be used

# --- Data copied from camera/field.hpp ---
WHITE_LINES_LENGTH = 1000
FIELD_X_SIZE = 312
FIELD_Y_SIZE = 253
GRID_SIZE = 1
WHITE_LINES = [
    [113, -5], [-156, -86], [153, -29], [-116, -48], [-154, 61],
    [154, -96], [-122, -126], [114, -48], [153, 24], [-156, -56],
    [-74, 125], [-156, -85], [-116, 34], [-22, -126], [-156, 86],
    [114, 35], [-156, -98], [-126, -126], [-154, -72], [-116, -8],
    [154, 59], [-156, 53], [-1, -125], [-156, -29], [113, -34],
    [-22, -125], [15, -126], [26, 125], [60, 123], [-142, -126],
    [-137, 125], [154, -126], [-135, 61], [116, 48], [114, 27],
    [132, -126], [-154, -104], [36, -126], [-154, -77], [74, -125],
    [154, 13], [-156, -70], [-154, -18], [-156, 94], [153, 32],
    [-154, -59], [-41, 123], [-118, -125], [-134, -125], [-143, -125],
    [-156, 93], [-156, -122], [-154, 75], [-156, 26], [-103, -126],
    [151, -62], [79, 125], [-41, 125], [25, 123], [114, 30],
    [-154, -126], [-2, -126], [-154, -35], [-156, 118], [153, 101],
    [-156, -78], [153, -93], [113, -11], [-154, -42], [114, 6],
    [154, 86], [154, -19], [154, -3], [-154, 6], [-84, -125],
    [-156, 24], [154, -34], [-78, 125], [-156, -114], [12, 125],
    [-54, -125], [-57, 125], [153, 120], [-145, 125], [14, 125],
    [154, 3], [154, -106], [113, -13], [-114, 6], [154, 37],
    [-6, -125], [116, -51], [-114, 14], [-156, 85], [-58, -126],
    [-116, 35], [-156, -115], [113, -24], [-156, -62], [79, -125],
    [-116, -32], [-49, -125], [-70, -126], [-154, 35], [114, -26],
    [14, -125], [1, -126], [153, -24], [74, 123], [153, -98],
    [87, 125], [62, 125], [153, -106], [-154, -48], [-9, 123],
    [-121, 125], [28, -126], [153, -62], [-154, -62], [6, 123],
    [-156, -14], [6, -125], [-140, -64], [-26, 123], [76, 125],
    [92, 123], [-156, -77], [26, 123], [63, 125], [-60, 123],
    [113, 125], [127, 59], [82, -125], [114, 21], [-154, 125],
    [122, 125], [-156, -43], [-156, 61], [-154, -64], [151, 62],
    [-154, 77], [153, 110], [23, -126], [46, 123], [114, -5],
    [-89, -126], [-156, 40], [-156, 67], [38, -125], [100, -125],
    [153, -43], [-38, 123], [153, -13], [-89, -125], [114, 8],
    [-154, -115], [-127, -126], [154, 110], [-156, -34], [-79, -125],
    [154, -67], [118, 123], [113, -2], [153, -61], [153, -10],
    [-122, -125], [84, -125], [-154, 2], [100, -126], [-150, -125],
    [153, 19], [154, 24], [-55, 123], [-154, 82], [153, -40],
    [98, -125], [-156, 77], [-154, -66], [-137, 62], [-156, -93],
    [114, -35], [153, 83], [153, 64], [153, 21], [-140, -62],
    [-111, -126], [-156, 64], [116, 125], [-6, 123], [-10, 125],
    [-138, -62], [-150, -64], [-154, -30], [-36, 123], [113, -14],
    [114, 18], [-100, 123], [38, 123], [47, -125], [154, 90],
    [113, -10], [-116, 3], [153, -67], [-105, -126], [49, -126],
    [-156, 10], [-154, 18], [30, 123], [122, -125], [113, 30],
    [60, -125], [-2, -125], [-92, 123], [-20, 125], [-130, -126],
    [-114, 2], [-63, 123], [132, -62], [-111, 125], [39, 125],
    [154, -91], [113, 14], [55, 123], [-58, 125], [-150, 61],
    [153, 82], [-156, 34], [153, -112], [-154, 13], [-86, -126],
    [-76, -125], [-114, 35], [153, -50], [-9, 125], [-18, -126],
    [154, 93], [119, 123], [65, 125], [-148, -62], [113, -30],
    [118, 125], [-156, 90], [-17, -126], [-154, -16], [-156, 50],
    [154, 48], [-103, -125], [-130, -62], [-156, 16], [114, -38],
    [110, -126], [-118, -50], [111, -125], [-119, 125], [70, -126],
    [63, 123], [1, 125], [114, 19], [-84, 125], [-97, 125],
    [-119, 53], [154, -58], [-156, 62], [-47, -126], [154, -61],
    [-154, -117], [-156, -112], [22, -125], [154, -75], [153, -58],
    [-114, -10], [-156, -118], [114, -46], [114, -45], [153, -82],
    [-23, 125], [-87, -125], [-154, -107], [-20, 123], [-39, -126],
    [-156, -126], [2, 123], [-156, 2], [-114, 22], [44, -126],
    [-156, -5], [-100, -125], [153, 93], [-156, 51], [140, -125],
    [-145, 123], [116, 50], [110, -125], [-116, -125], [-63, 125],
    [-116, 46], [154, -120], [-57, -126], [154, 96], [121, 56],
    [94, 125], [154, 29], [154, 21], [76, -126], [142, -62],
    [-25, 125], [-154, 56], [-154, 8], [-154, 112], [-116, -24],
    [-124, -126], [153, 56], [113, -18], [6, 125], [-154, -58],
    [-156, -74], [-108, 123], [-22, 125], [-154, 14], [154, 27],
    [153, -22], [-90, 123], [73, -125], [-26, 125], [-142, -62],
    [-121, 123], [-156, -59], [-150, -62], [113, -29], [-154, -45],
    [145, 61], [142, -126], [-34, -125], [-143, 123], [17, -126],
    [-111, 123], [-111, -125], [-44, 125], [-118, 125], [-1, -126],
    [-156, 101], [143, -125], [-132, 61], [4, 125], [-154, 94],
    [-156, -110], [153, 77], [-116, -34], [-52, 123], [-153, 125],
    [-156, 123], [114, -13], [153, 112], [-156, -40], [82, 123],
    [-150, 125], [143, -126], [-14, 125], [-156, -96], [-154, -114],
    [-114, 26], [23, -125], [124, 123], [-79, -126], [-153, -125],
    [153, 66], [103, -126], [114, 38], [-154, 117], [18, 125],
    [130, -62], [154, 123], [-156, 69], [-154, 45], [130, -126],
    [-100, -126], [-154, -85], [118, -54], [38, -126], [-118, 48],
    [-156, -54], [153, -118], [154, 122], [140, -64], [113, -32],
    [-134, 61], [-156, 110], [-54, 125], [50, -125], [2, -126],
    [-154, 27], [-12, -126], [-116, 5], [-137, -64], [154, 91],
    [114, -21], [153, -59], [-25, -125], [138, -62], [86, 125],
    [-154, -5], [114, -40], [137, 125], [-55, -125], [-154, 123],
    [-39, -125], [113, 11], [114, -19], [137, -125], [110, 123],
    [113, 3], [-52, -125], [113, 16], [-71, -126], [142, 62],
    [-116, -45], [-1, 125], [78, 123], [154, -69], [-87, -126],
    [154, -40], [114, -10], [89, -126], [92, 125], [73, 123],
    [-124, -125], [-60, -125], [153, 123], [154, 107], [-119, -126],
    [153, 46], [-135, 123], [20, 125], [-38, -125], [-116, -13],
    [-114, -126], [153, -8], [-154, -46], [81, 125], [-156, 88],
    [-71, 125], [-148, 123], [-114, -26], [113, 34], [114, 22],
    [-17, 123], [34, 123], [153, -2], [154, 120], [97, -126],
    [-154, -61], [-116, 2], [148, -64], [154, 16], [-116, 22],
    [153, -46], [-66, -126], [-105, 125], [153, -11], [113, -27],
    [-142, 61], [153, 75], [-154, -29], [-78, -126], [-145, 62],
    [153, 30], [-116, -18], [-46, -125], [34, -125], [-114, 19],
    [-154, 22], [135, 61], [-98, -125], [7, 125], [-130, 61],
    [-46, -126], [-156, -19], [-154, 72], [90, -125], [50, 123],
    [154, -42], [-148, -126], [-84, -126], [134, -126], [-1, 123],
    [143, -64], [154, -86], [94, -125], [-18, 123], [-108, -126],
    [33, 125], [65, -126], [114, -37], [52, -125], [-151, 62],
    [-154, -106], [-156, -94], [153, -125], [-154, 53], [129, -126],
    [113, -3], [111, 125], [127, -61], [-116, 14], [-154, -74],
    [150, 61], [-114, -11], [-114, 27], [-140, -125], [-156, -35],
    [-154, 42], [-36, 125], [146, -64], [-156, 3], [-156, 22],
    [151, -64], [-151, -125], [41, 123], [-156, -102], [-22, 123],
    [124, 58], [106, -126], [146, 61], [9, 125], [55, 125],
    [73, -126], [-143, -62], [-63, -126], [154, 0], [153, -83],
    [113, 32], [-42, 123], [4, 123], [154, 46], [-14, -125],
    [-116, 125], [-129, -126], [-156, 27], [-33, 125], [-156, 46],
    [134, 61], [-39, 123], [-65, 125], [65, 123], [-116, -3],
    [23, 123], [154, -88], [-114, -5], [-121, -125], [6, -126],
    [-73, 125], [-66, -125], [-114, -18], [-95, 125], [-154, 80],
    [-106, 123], [-138, 61], [-156, 54], [-154, -88], [148, 61],
    [-156, 66], [105, -126], [-95, 123], [-114, -13], [-134, 125],
    [124, -59], [-154, 98], [154, -85], [-113, 123], [-154, 64],
    [113, -26], [-156, -66], [-146, -62], [153, 98], [113, 19],
    [114, 29], [-116, 16], [153, 86], [-156, -16], [46, -126],
    [154, -78], [79, -126], [154, 61], [-25, -126], [-154, 110],
    [153, 62], [150, -64], [-154, 104], [-118, -51], [-156, 48],
    [-154, -67], [153, 42], [-156, 42], [105, 125], [140, -126],
    [-50, -125], [82, 125], [-114, 123], [114, 3], [-6, 125],
    [153, -54], [103, 123], [143, 123], [-114, 29], [135, -62],
    [-138, -64], [-116, 27], [153, -48], [-156, -107], [70, 125],
    [-156, -80], [-10, -125], [-114, 32], [114, -32], [153, -26],
    [102, 125], [-39, 125], [146, -126], [-68, -125], [97, 125],
    [-154, 29], [153, -117], [-154, 86], [153, 58], [100, 123],
    [153, -86], [62, -126], [154, -51], [-78, 123], [114, 43],
    [153, 18], [-154, -86], [-63, -125], [154, 88], [121, 123],
    [-26, -125], [47, 125], [103, -125], [-116, 32], [153, -6],
    [86, -125], [146, 125], [119, -126], [-42, -126], [78, 125],
    [126, -125], [55, -126], [153, 14], [153, 69], [39, 123],
    [-140, 125], [100, 125], [-154, 101], [-135, -62], [-154, -19],
    [116, -126], [154, -29], [-119, 51], [-132, 123], [-50, 125],
    [153, 104], [154, 6], [-102, -125], [153, -77], [113, -22],
    [118, -125], [153, -101], [113, 24], [-156, 112], [18, 123],
    [114, 40], [-151, -62], [-46, 125], [-154, 106], [113, -38],
    [-154, 0], [-94, -125], [153, -74], [153, -91], [121, -126],
    [113, 0], [-154, 37], [132, 125], [-153, -126], [154, 35],
    [-68, 125], [126, 59], [7, -125], [154, 14], [-154, 93],
    [142, -125], [-154, 58], [154, -8], [57, 125], [-114, -42],
    [114, 123], [-156, 19], [-81, 125], [154, 78], [-102, 123],
    [143, 61], [-7, 123], [153, -38], [-154, 114], [76, 123],
    [114, 13], [-132, -125], [-154, -118], [-119, -54], [154, -114],
    [135, -125], [154, 43], [-129, 125], [-154, -109], [-154, 66],
    [-62, 123], [47, 123], [62, 123], [-156, -26], [-154, 3],
    [-97, -126], [-89, 123], [-98, -126], [113, 10], [153, -18],
    [-154, 32], [154, -50], [-116, -35], [105, -125], [154, 5],
    [105, 123], [-81, -126], [153, 48], [-7, 125], [-106, 125],
    [121, 125], [153, 88], [-33, -126], [118, 53], [154, 66],
    [137, -64], [25, -126], [154, 75], [122, 58], [154, -14],
    [-138, 62], [-49, -126], [-154, 96], [-119, 123], [-6, -126],
    [-154, 40], [108, 125], [-116, 42], [79, 123], [-156, -46],
    [153, 59], [-2, 123], [-154, 83], [153, 43], [4, -126],
    [-66, 125], [-116, 11], [-143, -126], [154, 11], [-129, -61],
    [-28, -126], [-137, -125], [154, 19], [-118, 123], [17, -125],
    [-103, 123], [145, -125], [-154, -90], [148, -125], [154, -112],
    [113, 2], [-154, 10], [153, -51], [-132, 125], [-114, 24],
    [148, 125], [146, -62], [-116, -5], [-156, 75], [153, 6],
    [-15, -126], [1, 123], [-156, -88], [-154, -78], [114, 16],
    [-44, 123], [66, 123], [-142, -125], [-70, 123], [-110, 125],
    [58, 123], [113, -37], [-143, 62], [153, -80], [-12, 125],
    [153, 118], [-41, -126], [-156, -104], [102, 123], [153, 102],
    [127, -126], [138, -125], [-116, -11], [-156, 80], [153, -102],
    [108, 123], [-114, 8], [142, 61], [-154, 21], [127, 123],
    [-156, 115], [-146, -125], [68, -125], [-154, 115], [153, 27],
    [-23, -126], [-154, 59], [-23, 123], [-114, 3], [119, 125],
    [-154, 54], [97, -125], [153, 50], [-156, 120], [-148, -125],
    [-135, -126], [20, 123], [153, 35], [154, 54], [154, 58],
    [-65, 123], [130, -125], [-114, -37], [-31, -126], [134, 123],
    [39, -126], [153, 109], [-156, -30], [140, 123], [-156, -22],
    [-140, -126], [118, 51], [-156, -64], [-90, -125], [110, 125],
    [95, 125], [122, -59], [-90, 125], [113, 29], [54, 125],
    [-127, -61], [28, 123], [-146, 61], [114, -29], [95, 123],
    [71, -125], [150, 62], [-156, -3], [26, -126], [153, 10],
    [-142, 125], [49, 125], [-116, 123], [-116, -126], [-156, -61],
    [-154, -56], [-114, 40], [-154, -3], [42, -125], [74, 125],
    [-156, 104], [14, 123], [-71, 123], [-14, -126], [132, -125],
    [-148, -64], [135, -126], [-114, -22], [154, 125], [17, 125],
    [153, -42], [-114, -21], [154, 8], [153, 5], [-50, -126],
    [151, -125], [-126, -59], [154, -77], [-156, -75], [153, 8],
    [39, -125], [-116, -30], [154, 40], [154, 83], [154, -45],
    [153, 78], [-95, -126], [102, -126], [153, 22], [-154, -80],
    [-74, -126], [90, 125], [153, 51], [153, 40], [-126, 123],
    [-127, 125], [154, 45], [154, -101], [-116, 40], [-135, 125],
    [-126, -125], [154, -10], [154, 53], [-42, -125], [-154, 91],
    [-42, 125], [-31, 123], [154, -115], [47, -126], [-156, -37],
    [106, 125], [-114, 34], [-97, -125], [18, -125], [-154, 85],
    [-116, 37], [-154, -14], [154, -125], [113, 13], [-114, -8],
    [81, 123], [-143, -64], [-114, -6], [154, -53], [-151, -126],
    [-116, 18], [148, 62], [121, -125], [-154, -50], [137, 123],
    [-50, 123], [-110, -126], [-94, -126], [154, -24], [-116, -40],
    [-116, -42], [31, 123], [-146, 125], [130, 123], [-156, 102],
    [-116, 0], [12, 123], [114, 5], [31, -125], [114, -27],
    [-137, -126], [7, -126], [154, -26], [-156, -101], [-94, 125],
    [-121, 54], [-114, -14], [54, -126], [154, -109], [-114, -125],
    [154, -22], [84, 123], [-154, -2], [-156, 0], [-156, 109],
    [114, -14], [154, -98], [-114, -40], [153, 90], [-38, 125],
    [68, 125], [-34, -126], [-134, -126], [-114, 18], [-154, 51],
    [113, 123], [143, -62], [-138, -126], [154, -6], [145, -62],
    [41, 125], [-154, -83], [89, 125], [81, -126], [153, 107],
    [-154, 107], [-34, 123], [-30, -126], [-98, 125], [153, 80]
]
# --- End of copied data ---


def generate_ideal_image(guess_x: float, guess_y: float, guess_heading: float,
                         img_width: int, img_height: int) -> np.ndarray:
    """
    Generates an ideal black and white image based on the projected
    white line coordinates for a given guess pose.

    Mimics the transformation logic in the C++ calculate_loss function.

    Args:
        guess_x: The x offset component of the guess.
        guess_y: The y offset component of the guess.
        guess_heading: The heading component (in radians) of the guess.
        img_width: The width of the output image.
        img_height: The height of the output image.

    Returns:
        A numpy array (H, W) representing the black and white image (uint8).
        White (255) pixels correspond to projected line points.
    """
    # Create a black image (H, W format using numpy)
    # Using uint8 for standard image format, 0=black, 255=white
    ideal_image = np.zeros((img_height, img_width), dtype=np.uint8)

    # Pre-calculate sin and cos
    sin_theta = math.sin(guess_heading)
    cos_theta = math.cos(guess_heading)

    # Center of the image
    img_center_x = img_width / 2.0
    img_center_y = img_height / 2.0

    count_in_bounds = 0

    # Transform & rotate white line coords
    for i in range(WHITE_LINES_LENGTH):
        # Original field coordinates of the white line point
        x = WHITE_LINES[i][0]
        y = WHITE_LINES[i][1]

        # Transform to relative coordinates (matching C++: x + guess.x)
        # Note: This assumes guess.x/y are offsets *added* to field coords.
        rel_x = float(x + guess_x)
        rel_y = float(y + guess_y)

        # Rotate using floating-point arithmetic
        rotated_x = rel_x * cos_theta - rel_y * sin_theta
        rotated_y = rel_x * sin_theta + rel_y * cos_theta

        # Convert back to integer coordinate space with offset (like C++)
        # Rounding might be slightly different than C++ int conversion, but close.
        # Using // for integer division for the offset part.
        final_x_cpp = round(rotated_x) + img_width // 2
        final_y_cpp = round(rotated_y) + img_height // 2

        # Check if the point is within IMAGE boundaries (using the C++ variable names)
        if (final_x_cpp < 0 or final_x_cpp >= img_width or
                final_y_cpp < 0 or final_y_cpp >= img_height):
            continue

        # Calculate the *actual* pixel coordinates accessed by the C++ code
        # C++ code: camera_image.ptr<cv::Vec3b>(final_x)[IMG_HEIGHT - final_y]
        # This implies: image_y (row) = final_x_cpp
        #              image_x (col) = img_height - final_y_cpp
        # This is unusual, but we replicate it.
        image_y = int(final_x_cpp)
        image_x = int(img_height - final_y_cpp) # Note the flip and axis swap!

        # Check if these *derived* pixel coordinates are valid for the image array
        # Need to check image_y against height and image_x against width
        if 0 <= image_y < img_height and 0 <= image_x < img_width:
             # Set the corresponding pixel to white (255)
             # Numpy uses [row, col] = [y, x]
             ideal_image[image_y, image_x] = 255
             count_in_bounds += 1
        # else:
            # This case happens if final_x/y were in C++ bounds, but the
            # derived image_x/y (due to the unusual C++ indexing) are not.
            # print(f"Warning: Point ({x},{y}) -> final_cpp ({final_x_cpp},{final_y_cpp}) "
            #       f"-> pixel ({image_x},{image_y}) is out of bounds after weird indexing.")


    print(f"Generated ideal image. {count_in_bounds} white line points plotted.")
    return ideal_image

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate an ideal black and white image based on projected field lines, "
                    "mimicking the C++ calculate_loss transformation."
    )
    parser.add_argument("guess_x", type=float, help="X component of the guess/offset.")
    parser.add_argument("guess_y", type=float, help="Y component of the guess/offset.")
    parser.add_argument("guess_heading", type=float, help="Heading component of the guess (in radians).")
    parser.add_argument("--width", type=int, default=320, help="Width of the output image (IMG_WIDTH).")
    parser.add_argument("--height", type=int, default=240, help="Height of the output image (IMG_HEIGHT).")
    parser.add_argument("-o", "--output", type=str, default="ideal_image.png", help="Output image filename.")

    args = parser.parse_args()

    # Generate the image
    ideal_img = generate_ideal_image(
        args.guess_x, args.guess_y, args.guess_heading,
        args.width, args.height
    )

    # Save the image
    if cv2.imwrite(args.output, ideal_img):
        print(f"Ideal image saved to {args.output}")
    else:
        print(f"Error saving image to {args.output}")

    # Optional: Display the image
    # cv2.imshow("Ideal Image", ideal_img)
    # cv2.waitKey(0)
    # cv2.destroyAllWindows()