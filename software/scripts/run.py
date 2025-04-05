#!/usr/bin/env python3
# filepath: button_control.py

import RPi.GPIO as GPIO
import time
import subprocess
import signal
import os

# Configuration
BUTTON_PIN = 6  # GPIO6 for button
EXECUTABLE_PATH = "./main"  # Replace with your executable path
DEBOUNCE_TIME = 0.3  # Seconds for debounce

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

process = None
press_count = 0
last_press_time = 0

def cleanup():
    """Clean up GPIO and kill process if running"""
    if process:
        try:
            os.killpg(os.getpgid(process.pid), signal.SIGKILL)
        except:
            pass
    GPIO.cleanup()
    print("Cleanup complete")

try:
    print("Button control script started. Press GPIO6 button...")
    
    while True:
        # Wait for button press (detect falling edge when button is pressed)
        if GPIO.input(BUTTON_PIN) == GPIO.LOW:
            current_time = time.time()
            
            # Debounce
            if current_time - last_press_time > DEBOUNCE_TIME:
                press_count += 1
                last_press_time = current_time
                print(f"Button press #{press_count} detected")
                
                if press_count % 2 == 1:  # Odd press
                    if process is None or process.poll() is not None:
                        print("Starting executable...")
                        # Start process in its own process group for proper signal handling
                        process = subprocess.Popen(
                            EXECUTABLE_PATH, 
                            shell=True, 
                            preexec_fn=os.setsid
                        )
                        print(f"Process started with PID: {process.pid}")
                    else:
                        print("Process already running")
                        
                else:  # Even press
                    if process and process.poll() is None:
                        print("Sending SIGINT to process...")
                        os.killpg(os.getpgid(process.pid), signal.SIGINT)
                        
                        print("Waiting 5 seconds before termination...")
                        start_wait = time.time()
                        while time.time() - start_wait < 5:
                            if process.poll() is not None:
                                print("Process exited after SIGINT")
                                break
                            time.sleep(0.1)
                        
                        # If process still running after 5s, terminate it
                        if process.poll() is None:
                            print("Process still running after 5s, terminating...")
                            os.killpg(os.getpgid(process.pid), signal.SIGTERM)
                    else:
                        print("No running process to terminate")
            
            # Wait for button release to avoid multiple triggers
            while GPIO.input(BUTTON_PIN) == GPIO.LOW:
                time.sleep(0.01)
                
        time.sleep(0.01)  # Reduce CPU usage

except KeyboardInterrupt:
    print("Script interrupted by user")
finally:
    cleanup()
