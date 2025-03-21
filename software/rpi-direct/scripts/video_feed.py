import io
import logging
import socketserver
from threading import Condition, Thread
from http import server

from picamera2 import Picamera2
from picamera2.encoders import JpegEncoder
from picamera2.outputs import FileOutput

# HTML page template for streaming
PAGE = """\
<html>
<head>
<title>Raspberry Pi Camera Stream</title>
</head>
<body>
<h1>Raspberry Pi Camera Stream</h1>
<img src="stream.mjpg" width="640" height="480" />
</body>
</html>
"""

class StreamingOutput(io.BufferedIOBase):
    def __init__(self):
        self.frame = None
        self.condition = Condition()

    def write(self, buf):
        with self.condition:
            self.frame = buf
            self.condition.notify_all()
        return len(buf)

class StreamingHandler(server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(PAGE))
            self.end_headers()
            self.wfile.write(PAGE.encode('utf-8'))
        elif self.path == '/stream.mjpg':
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

if __name__ == '__main__':
    try:
        # Set up logging
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        
        # Initialize camera output
        output = StreamingOutput()
        
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