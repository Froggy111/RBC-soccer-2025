#!/usr/bin/env python3

import RPi.GPIO as GPIO
import subprocess
import time
import signal
import sys

# Configuration
BUTTON_PIN = 31        # GPIO pin for button
EXECUTABLE = "./main"  # Primary executable 
DESTROY = "./destroy"  # Secondary executable
DEBOUNCE_TIME = 0.3    # Debounce time in seconds

# Setup
process = None
current_app = None  # Tracks which app is currently running

def setup():
    GPIO.setmode(GPIO.BOARD)  # Use physical pin numbering
    GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)  # Set pin as input with pull-up

def toggle_executable():
    global process, current_app
    
    # Stop any currently running process
    if process:
        process.terminate()
        try:
            process.wait(timeout=5)  # Wait up to 5 seconds for normal termination
        except subprocess.TimeoutExpired:
            process.kill()  # Force kill if it doesn't terminate
        process = None
        print(f"{current_app} stopped")
    
    # Toggle between executables
    if current_app == EXECUTABLE or current_app is None:
        # Switch to destroy
        try:
            process = subprocess.Popen(DESTROY, shell=True)
            current_app = DESTROY
            print("Destroy executable started")
        except Exception as e:
            print(f"Error starting destroy executable: {e}")
            current_app = None
    else:
        # Switch to main
        try:
            process = subprocess.Popen(EXECUTABLE, shell=True)
            current_app = EXECUTABLE
            print("Main executable started")
        except Exception as e:
            print(f"Error starting main executable: {e}")
            current_app = None

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