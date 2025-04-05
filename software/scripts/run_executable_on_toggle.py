#!/usr/bin/env python3
# filepath: button_toggle.py

import RPi.GPIO as GPIO
import subprocess
import time
import signal
import sys

# Configuration
BUTTON_PIN = 31        # GPIO pin for button
EXECUTABLE = "./your_executable"  # Change this to your executable path
DEBOUNCE_TIME = 0.3    # Debounce time in seconds

# Setup
process = None
running = False

def setup():
    GPIO.setmode(GPIO.BOARD)  # Use physical pin numbering
    GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)  # Set pin as input with pull-up

def toggle_executable():
    global process, running
    
    if running:
        # Stop the executable
        if process:
            process.terminate()
            try:
                process.wait(timeout=5)  # Wait up to 5 seconds for normal termination
            except subprocess.TimeoutExpired:
                process.kill()  # Force kill if it doesn't terminate
            process = None
        running = False
        print("Executable stopped")
    else:
        # Start the executable
        try:
            process = subprocess.Popen(EXECUTABLE, shell=True)
            running = True
            print("Executable started")
        except Exception as e:
            print(f"Error starting executable: {e}")

def cleanup():
    # Ensure we stop the process when exiting
    if process:
        process.terminate()
        try:
            process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            process.kill()
    GPIO.cleanup()
    print("Cleanup done")

def main():
    setup()
    
    # Register cleanup handler for Ctrl+C
    signal.signal(signal.SIGINT, lambda sig, frame: (cleanup(), sys.exit(0)))
    
    last_button_state = GPIO.input(BUTTON_PIN)
    last_press_time = 0
    
    print("Monitoring button presses. Press Ctrl+C to exit.")
    
    try:
        while True:
            current_button_state = GPIO.input(BUTTON_PIN)
            
            # Button press detected (falling edge - button connects to ground when pressed)
            if current_button_state == 0 and last_button_state == 1:
                # Debounce
                current_time = time.time()
                if current_time - last_press_time > DEBOUNCE_TIME:
                    toggle_executable()
                    last_press_time = current_time
                    
            last_button_state = current_button_state
            time.sleep(0.01)  # Small delay to reduce CPU usage
            
    finally:
        cleanup()

if __name__ == "__main__":
    main()