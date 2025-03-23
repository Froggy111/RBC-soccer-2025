import logging
import os
from datetime import datetime
from picamera2 import Picamera2
from picamera2.encoders import H264Encoder
from picamera2.outputs import FileOutput

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

class RecordingController:
    def __init__(self, output_dir="recordings"):
        self.picam2 = None
        self.encoder = None
        self.output_file = None
        self.is_recording = False
        self.output_dir = output_dir
        
        # Create output directory if it doesn't exist
        os.makedirs(self.output_dir, exist_ok=True)
        
        # Initialize camera
        self.initialize_camera()
    
    def initialize_camera(self):
        # Initialize Picamera2
        self.picam2 = Picamera2()
        
        # Configure the camera
        config = self.picam2.create_video_configuration(
            main={"size": (1280, 720)}
        )
        self.picam2.configure(config)
        self.picam2.start()
        logging.info("Camera initialized")
    
    def start_recording(self):
        if self.is_recording:
            logging.warning("Already recording")
            return False
        
        try:
            # Generate filename with timestamp
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            self.output_file = os.path.join(self.output_dir, f"video_{timestamp}.h264")
            
            # Create encoder
            self.encoder = H264Encoder(bitrate=10000000)
            
            # Start recording
            self.picam2.start_recording(self.encoder, FileOutput(self.output_file))
            
            self.is_recording = True
            logging.info(f"Started recording to {self.output_file}")
            return True
        except Exception as e:
            logging.error(f"Error starting recording: {str(e)}")
            return False
    
    def stop_recording(self):
        if not self.is_recording:
            logging.warning("Not currently recording")
            return False
        
        try:
            self.picam2.stop_recording()
            self.is_recording = False
            logging.info(f"Stopped recording. Video saved to {self.output_file}")
            return True
        except Exception as e:
            logging.error(f"Error stopping recording: {str(e)}")
            return False
    
    def cleanup(self):
        if self.is_recording:
            self.stop_recording()
        
        if self.picam2:
            self.picam2.close()
            logging.info("Camera resources released")

def main():
    controller = RecordingController()
    
    try:
        while True:
            command = input("Enter command (start, stop, quit): ").strip().lower()
            
            if command == "start":
                controller.start_recording()
            elif command == "stop":
                controller.stop_recording()
            elif command == "quit":
                break
            else:
                print("Unknown command. Available commands: start, stop, quit")
    
    except KeyboardInterrupt:
        logging.info("Keyboard interrupt received. Stopping...")
    finally:
        controller.cleanup()
        logging.info("Program ended")

if __name__ == '__main__':
    main()