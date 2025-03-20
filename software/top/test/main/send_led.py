#!/usr/bin/env python3
import serial
import time
import struct
import random

class RP2040Communicator:
    """Library to communicate with RP2040 using the defined USB protocol"""
    
    # From include/comms/identifiers.hpp
    class RecvIdentifiers:
        LEDs = 1
    
    def __init__(self, port="/dev/ttyACM1", baudrate=115200, timeout=1):
        """Initialize the serial connection to the RP2040"""
        try:
            self.ser = serial.Serial(port, baudrate, timeout=timeout)
            print(f"Connected to {port}")
            time.sleep(2)  # Give time for connection to stabilize
        except serial.SerialException as e:
            print(f"Error opening serial port: {e}")
            raise
    
    def send_command(self, identifier, data):
        """
        Send a command following the protocol:
        - 2 bytes: Length (little-endian, includes identifier byte)
        - 1 byte: Identifier 
        - Remaining bytes: Data
        """
        # Calculate length (including identifier byte)
        length = len(data) + 1
        
        # Pack length as little-endian 16-bit value
        length_bytes = struct.pack('<H', length)
        
        # Pack identifier as 8-bit value
        identifier_byte = struct.pack('<B', identifier)
        
        # Combine and send
        packet = length_bytes + identifier_byte + data
        self.ser.write(packet)
        
    def send_led(self, led_index, r, g, b):
        data = struct.pack('<BBBB', led_index, r, g, b)
        self.send_command(self.RecvIdentifiers.LEDs, data)

    
    def update_led(self):
        """
        Continuously update a single LED with random colors every second
        """
        try:
            while True:
                led_index = random.randint(0, 16)
                r = 0
                g = 0
                b = 254
                
                print(f"Updating LED {led_index} with RGB({r},{g},{b})")
                self.send_led(led_index, r, g, b)
                
                # Wait 1 second between updates
                time.sleep(0.4)
        except KeyboardInterrupt:
            print("LED update interrupted")
        finally:
            # Clean close of serial connection
            self.ser.close()
            print("Serial connection closed")
    
    def close(self):
        """Close the serial connection"""
        if hasattr(self, 'ser') and self.ser.is_open:
            self.ser.close()
            print("Serial connection closed")

if __name__ == "__main__":
    try:
        # Create the communicator
        comm = RP2040Communicator()
        
        comm.update_led()
    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Make sure to close the serial connection
        if 'comm' in locals():
            comm.close()