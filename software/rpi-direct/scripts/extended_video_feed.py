import io
import logging
import socketserver
from threading import Condition, Thread, Lock
from http import server
import cv2
import numpy as np
from PIL import Image
import urllib.parse

from picamera2 import Picamera2
from picamera2.encoders import JpegEncoder
from picamera2.outputs import FileOutput

# HTML page template with controls for selecting the processing method
PAGE = """\
<html>
<head>
<title>Raspberry Pi Camera Stream</title>
<style>
    body {
        font-family: Arial, sans-serif;
        max-width: 1200px;
        margin: 0 auto;
        padding: 20px;
    }
    .stream-container {
        margin-top: 20px;
    }
    .controls {
        margin-bottom: 20px;
        padding: 15px;
        background-color: #f5f5f5;
        border-radius: 5px;
    }
    select, input {
        margin: 5px;
        padding: 5px;
    }
    button {
        background-color: #4CAF50;
        border: none;
        color: white;
        padding: 8px 16px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 14px;
        margin: 4px 2px;
        cursor: pointer;
        border-radius: 4px;
    }
    .param-group {
        margin-top: 10px;
        padding: 10px;
        border: 1px solid #ddd;
        border-radius: 4px;
        display: none;
    }
</style>
<script>
    function updateParams() {
        const method = document.getElementById('method').value;
        document.querySelectorAll('.param-group').forEach(group => {
            group.style.display = 'none';
        });
        document.getElementById(method + '-params').style.display = 'block';
    }
    
    function applySettings() {
        const method = document.getElementById('method').value;
        let params = {};
        
        // Common parameters
        params.method = method;
        
        // Get parameters specific to each method
        if (method === 'none') {
            // No parameters needed
        }
        else if (method === 'unwrap') {
            params.center_x = document.getElementById('unwrap-center-x').value;
            params.center_y = document.getElementById('unwrap-center-y').value;
            params.max_radius = document.getElementById('unwrap-max-radius').value;
            params.unwrap_height = document.getElementById('unwrap-height').value;
            params.unwrap_width = document.getElementById('unwrap-width').value;
        }
        else if (method === 'unwrap_shrink') {
            params.center_x = document.getElementById('unwrap-shrink-center-x').value;
            params.center_y = document.getElementById('unwrap-shrink-center-y').value;
            params.max_radius = document.getElementById('unwrap-shrink-max-radius').value;
            params.unwrap_height = document.getElementById('unwrap-shrink-height').value;
            params.unwrap_width = document.getElementById('unwrap-shrink-width').value;
            params.shrink_coeff = document.getElementById('shrink-coeff').value;
        }
        else if (method === 'shrink_direct') {
            params.shrink_factor = document.getElementById('shrink-factor').value;
        }
        
        // Send the parameters to the server
        fetch('/set_params?' + new URLSearchParams(params).toString())
            .then(response => response.text())
            .then(data => {
                console.log(data);
                document.getElementById('status').innerHTML = 'Settings applied: ' + data;
            });
    }
    
    window.onload = function() {
        updateParams();
    }
</script>
</head>
<body>
<h1>Raspberry Pi Camera Stream</h1>

<div class="controls">
    <h3>Image Processing Controls</h3>
    
    <div>
        <label for="method">Processing Method:</label>
        <select id="method" onchange="updateParams()">
            <option value="none">None (Raw Feed)</option>
            <option value="unwrap">Unwrap</option>
            <option value="unwrap_shrink">Unwrap with Shrink</option>
            <option value="shrink_direct">Direct Shrink</option>
        </select>
    </div>
    
    <div id="none-params" class="param-group">
        <p>No parameters needed for raw feed.</p>
    </div>
    
    <div id="unwrap-params" class="param-group">
        <h4>Unwrap Parameters</h4>
        <div>
            <label for="unwrap-center-x">Center X:</label>
            <input type="number" id="unwrap-center-x" value="320" />
        </div>
        <div>
            <label for="unwrap-center-y">Center Y:</label>
            <input type="number" id="unwrap-center-y" value="240" />
        </div>
        <div>
            <label for="unwrap-max-radius">Max Radius:</label>
            <input type="number" id="unwrap-max-radius" value="240" />
        </div>
        <div>
            <label for="unwrap-height">Output Height:</label>
            <input type="number" id="unwrap-height" value="300" />
        </div>
        <div>
            <label for="unwrap-width">Output Width:</label>
            <input type="number" id="unwrap-width" value="800" />
        </div>
    </div>
    
    <div id="unwrap_shrink-params" class="param-group">
        <h4>Unwrap with Shrink Parameters</h4>
        <div>
            <label for="unwrap-shrink-center-x">Center X:</label>
            <input type="number" id="unwrap-shrink-center-x" value="320" />
        </div>
        <div>
            <label for="unwrap-shrink-center-y">Center Y:</label>
            <input type="number" id="unwrap-shrink-center-y" value="240" />
        </div>
        <div>
            <label for="unwrap-shrink-max-radius">Max Radius:</label>
            <input type="number" id="unwrap-shrink-max-radius" value="240" />
        </div>
        <div>
            <label for="unwrap-shrink-height">Unwrap Height:</label>
            <input type="number" id="unwrap-shrink-height" value="300" />
        </div>
        <div>
            <label for="unwrap-shrink-width">Unwrap Width:</label>
            <input type="number" id="unwrap-shrink-width" value="800" />
        </div>
        <div>
            <label for="shrink-coeff">Shrink Coefficient:</label>
            <input type="number" id="shrink-coeff" value="0.7" step="0.1" min="0" max="1" />
        </div>
    </div>
    
    <div id="shrink_direct-params" class="param-group">
        <h4>Direct Shrink Parameters</h4>
        <div>
            <label for="shrink-factor">Shrink Factor:</label>
            <input type="number" id="shrink-factor" value="-0.3" step="0.1" min="-1" max="1" />
        </div>
    </div>
    
    <button onclick="applySettings()">Apply Settings</button>
    <div id="status"></div>
</div>

<div class="stream-container">
    <img src="stream.mjpg" width="800" />
</div>
</body>
</html>
"""

# ----- Image Processing Functions -----

def unwrap_image(image, center=(320, 240), max_radius=240, output_size=(300, 800)):
    """
    Unwrap a fisheye image into a panoramic view.
    
    Args:
        image: Input image (OpenCV format)
        center: Center point of the fisheye lens (x, y)
        max_radius: Maximum radius of the fisheye lens
        output_size: Size of the output image (height, width)
        
    Returns:
        Unwrapped image
    """
    height, width = output_size
    unwrapped = np.zeros((height, width, 3), dtype=np.uint8)
    
    # For each pixel in the output image
    for y in range(height):
        for x in range(width):
            # Convert to polar coordinates
            theta = 2 * np.pi * x / width
            radius = max_radius * (1 - y / height)
            
            # Convert to Cartesian coordinates relative to center
            src_x = int(center[0] + radius * np.cos(theta))
            src_y = int(center[1] + radius * np.sin(theta))
            
            # Check if within bounds of source image
            if 0 <= src_x < image.shape[1] and 0 <= src_y < image.shape[0]:
                unwrapped[y, x] = image[src_y, src_x]
    
    return unwrapped

def vertical_shrink(image, coefficient=0.7):
    """
    Apply vertical shrink to an image.
    
    Args:
        image: Input image
        coefficient: Shrink coefficient (0-1)
        
    Returns:
        Shrunk image
    """
    height, width = image.shape[:2]
    result = np.zeros_like(image)
    
    # For each column in the image
    for x in range(width):
        # Calculate position in source image
        for y in range(height):
            # Apply shrink: compress the middle more than the edges
            normalized_y = 2 * y / height - 1  # -1 to 1
            factor = 1 - coefficient * (1 - normalized_y**2)
            src_y = int(height * (normalized_y * factor + 1) / 2)
            
            src_y = max(0, min(src_y, height - 1))
            result[y, x] = image[src_y, x]
    
    return result

def center_shrink(image, shrink_factor=0.3):
    """
    Apply center-based shrink/expand effect to an image.
    
    Args:
        image: Input image (OpenCV format)
        shrink_factor: Positive values expand center, negative values shrink
        
    Returns:
        Processed image
    """
    if isinstance(image, np.ndarray):
        # Convert OpenCV to PIL
        if image.shape[2] == 3:
            pil_image = Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
        else:
            pil_image = Image.fromarray(image)
    else:
        # Assume it's already PIL or a path
        pil_image = Image.open(image) if isinstance(image, str) else image
        
    input_width, input_height = pil_image.size
    img_array = np.array(pil_image)
    
    # Set output dimensions
    output_width = input_width
    output_height = input_height
    
    # Create coordinate grids for the output image
    y_dest, x_dest = np.mgrid[0:output_height, 0:output_width]
    
    # Calculate center coordinates of the output image
    center_y, center_x = output_height // 2, output_width // 2
    
    # Calculate distance from center (normalized to [0, 1])
    max_distance = np.sqrt(max(center_x, center_y)**2 + max(center_x, center_y)**2)
    distance = np.sqrt((x_dest - center_x)**2 + (y_dest - center_y)**2) / max_distance
    
    # Calculate the shrink factor based on distance
    shrink_map = 1.0 - shrink_factor * (1.0 - distance)
    shrink_map = np.maximum(shrink_map, 0.001)  # Avoid division by zero
    
    # Calculate source coordinates
    source_x = center_x + (x_dest - center_x) / shrink_map
    source_y = center_y + (y_dest - center_y) / shrink_map
    
    # Create a mask for valid coordinates
    valid_coords = (
        (source_x >= 0) & 
        (source_x < input_width - 1) &
        (source_y >= 0) & 
        (source_y < input_height - 1)
    )
    
    # For valid pixels, round to nearest integer for sampling
    source_x_valid = np.round(source_x[valid_coords]).astype(int)
    source_y_valid = np.round(source_y[valid_coords]).astype(int)
    
    # Create output image with proper dimensions
    if len(img_array.shape) == 3:  # RGB/RGBA image
        channels = img_array.shape[2]
        
        # Create transparent background if RGBA image
        if channels == 4:
            output = np.zeros((output_height, output_width, channels), dtype=img_array.dtype)
            output[:, :, 3] = 0  # Set alpha channel to fully transparent
        else:
            output = np.zeros((output_height, output_width, channels), dtype=img_array.dtype)
        
        # Map pixels from source to destination for valid coordinates
        for c in range(channels):
            output[y_dest[valid_coords], x_dest[valid_coords], c] = img_array[source_y_valid, source_x_valid, c]
    else:  # Grayscale image
        output = np.zeros((output_height, output_width), dtype=img_array.dtype)
        output[y_dest[valid_coords], x_dest[valid_coords]] = img_array[source_y_valid, source_x_valid]
    
    # Convert back to OpenCV format
    if len(output.shape) == 3 and output.shape[2] == 3:
        output = cv2.cvtColor(output, cv2.COLOR_RGB2BGR)
        
    return output

def unwrap_image_shrink(image, center=(320, 240), max_radius=240, 
                        output_size=(300, 800), shrink_coeff=0.7):
    """
    Unwrap a fisheye image and apply vertical shrink.
    
    Args:
        image: Input image (OpenCV format)
        center: Center point of the fisheye lens (x, y)
        max_radius: Maximum radius of the fisheye lens
        output_size: Size of the output image (height, width)
        shrink_coeff: Coefficient for vertical shrinking (0-1)
        
    Returns:
        Processed image
    """
    # First unwrap the image
    unwrapped = unwrap_image(image, center, max_radius, output_size)
    
    # Then apply vertical shrink
    result = vertical_shrink(unwrapped, shrink_coeff)
    
    return result

# ----- Image Processor Class -----

class ImageProcessor:
    def __init__(self):
        self.method = "none"
        self.params = {
            "unwrap": {
                "center_x": 320,
                "center_y": 240,
                "max_radius": 240,
                "unwrap_height": 300,
                "unwrap_width": 800
            },
            "unwrap_shrink": {
                "center_x": 320,
                "center_y": 240,
                "max_radius": 240,
                "unwrap_height": 300,
                "unwrap_width": 800,
                "shrink_coeff": 0.7
            },
            "shrink_direct": {
                "shrink_factor": -0.3
            }
        }
        self.lock = Lock()
    
    def set_method(self, method):
        with self.lock:
            self.method = method
    
    def update_params(self, method, new_params):
        with self.lock:
            for key, value in new_params.items():
                if key in self.params.get(method, {}):
                    try:
                        self.params[method][key] = float(value)
                    except ValueError:
                        pass
    
    def process_frame(self, frame):
        with self.lock:
            method = self.method
            if method == "none":
                return frame
                
            if method == "unwrap":
                params = self.params["unwrap"]
                center = (int(params["center_x"]), int(params["center_y"]))
                max_radius = int(params["max_radius"])
                output_size = (int(params["unwrap_height"]), int(params["unwrap_width"]))
                
                result = unwrap_image(frame, center, max_radius, output_size)
                return result
                
            elif method == "unwrap_shrink":
                params = self.params["unwrap_shrink"]
                center = (int(params["center_x"]), int(params["center_y"]))
                max_radius = int(params["max_radius"])
                output_size = (int(params["unwrap_height"]), int(params["unwrap_width"]))
                shrink_coeff = float(params["shrink_coeff"])
                
                result = unwrap_image_shrink(frame, center, max_radius, output_size, shrink_coeff)
                return result
                
            elif method == "shrink_direct":
                params = self.params["shrink_direct"]
                shrink_factor = float(params["shrink_factor"])
                
                result = center_shrink(frame, shrink_factor)
                return result
            
            return frame

# ----- Streaming Classes -----

class StreamingOutput(io.BufferedIOBase):
    def __init__(self, processor):
        self.frame = None
        self.condition = Condition()
        self.processor = processor

    def write(self, buf):
        with self.condition:
            # Convert the buffer to a numpy array
            data = np.frombuffer(buf, dtype=np.uint8)
            # Decode the JPEG
            frame = cv2.imdecode(data, cv2.IMREAD_COLOR)
            
            if frame is not None:
                # Process the frame
                processed_frame = self.processor.process_frame(frame)
                
                # Encode back to JPEG
                ret, encoded_frame = cv2.imencode('.jpg', processed_frame)
                if ret:
                    self.frame = encoded_frame.tobytes()
                    self.condition.notify_all()
            
        return len(buf)

class StreamingHandler(server.BaseHTTPRequestHandler):
    def do_GET(self):
        path = self.path
        if path == '/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(PAGE))
            self.end_headers()
            self.wfile.write(PAGE.encode('utf-8'))
        elif path.startswith('/set_params'):
            # Parse query parameters
            query = urllib.parse.urlparse(path).query
            params = urllib.parse.parse_qs(query)
            
            # Convert lists to single values
            params = {k: v[0] for k, v in params.items()}
            
            # Update processor settings
            method = params.get('method', 'none')
            processor.set_method(method)
            
            if method in ['unwrap', 'unwrap_shrink', 'shrink_direct']:
                processor.update_params(method, params)
            
            # Send response
            response = f"Method set to {method} with parameters"
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.send_header('Content-Length', len(response))
            self.end_headers()
            self.wfile.write(response.encode('utf-8'))
        elif path == '/stream.mjpg':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            try:
                while True:
                    with output.condition:
                        output.condition.wait()
                        frame = output.frame
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', len(frame))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
            except Exception as e:
                logging.warning(
                    'Removed streaming client %s: %s',
                    self.client_address, str(e))
        else:
            self.send_error(404)
            self.end_headers()

class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

def start_camera(output):
    # Initialize Picamera2
    picam2 = Picamera2()
    
    # Configure the camera
    config = picam2.create_video_configuration(
        main={"size": (640, 480)}, 
        lores={"size": (320, 240)}, 
        display="lores"
    )
    picam2.configure(config)
    
    # Create encoder and start encoding
    encoder = JpegEncoder(q=70)
    picam2.start_recording(encoder, FileOutput(output))

# ----- Main Application -----

if __name__ == '__main__':
    try:
        # Set up logging
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        
        # Initialize image processor
        processor = ImageProcessor()
        
        # Initialize camera output with processor
        output = StreamingOutput(processor)
        
        # Start camera in a separate thread
        camera_thread = Thread(target=start_camera, args=(output,))
        camera_thread.daemon = True
        camera_thread.start()
        
        # Get the private IP address of the Pi
        import socket
        hostname = socket.gethostname()
        ip_address = socket.gethostbyname(hostname)
        
        # Start the server
        port = 8000
        address = ('', port)  # Empty string means all available interfaces
        server = StreamingServer(address, StreamingHandler)
        logging.info(f"Server started at http://{ip_address}:{port}")
        logging.info("Press Ctrl+C to stop the server")
        server.serve_forever()
            
    except KeyboardInterrupt:
        logging.info("Keyboard interrupt received. Stopping...")
    except Exception as e:
        logging.error(f"Error: {str(e)}")
    finally:
        # If picam2 exists in the scope, stop recording
        if 'picam2' in locals():
            try:
                picam2.stop_recording()
            except:
                pass
        logging.info("Stream ended")