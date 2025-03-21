#!/usr/bin/env python3
import serial
import struct
import time
import argparse
import logging
import os
from enum import IntEnum
from datetime import datetime

class SendIdentifiers(IntEnum):
    """Enum values from include/comms/identifiers.hpp"""
    COMMS_WARN = 0
    COMMS_ERROR = 1
    COMMS_DEBUG = 2
    SPI_FAIL = 3
    LED_LISTENER_FAIL = 4
    ICM29048 = 5

class CommsErrors(IntEnum):
    """Enum values from include/comms/errors.hpp"""
    PACKET_RECV_OVER_MAX_BUFSIZE = 0
    PACKET_SEND_TOO_LONG = 1
    PACKET_RECV_OVER_COMMAND_LISTENER_MAXSIZE = 2
    ATTACH_LISTENER_NULLPTR = 3
    CALLING_UNATTACHED_LISTENER = 4
    LISTENER_NO_BUFFER = 5
    LISTENER_NO_BUFFER_MUTEX = 6

class CommsWarnings(IntEnum):
    """Enum values from include/comms/errors.hpp"""
    OVERRIDE_COMMAND_LISTENER = 0
    LISTENER_BUFFER_MUTEX_HELD = 1

def setup_logger(log_file, log_level):
    """Set up logger to write to both console and file"""
    # Create logger
    logger = logging.getLogger("usb_receiver")
    logger.setLevel(log_level)
    
    # Create formatter
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    
    # Create console handler
    console_handler = logging.StreamHandler()
    console_handler.setLevel(log_level)
    console_handler.setFormatter(formatter)
    
    # Create file handler
    file_handler = logging.FileHandler(log_file)
    file_handler.setLevel(log_level)
    file_handler.setFormatter(formatter)
    
    # Add handlers to logger
    logger.addHandler(console_handler)
    logger.addHandler(file_handler)
    
    return logger

def parse_comms_error(data):
    """Parse error data based on CommsErrors enum"""
    if len(data) >= 1:
        error_code = data[0]
        try:
            error_name = CommsErrors(error_code).name
            additional_info = f"Additional data: {data[1:]}" if len(data) > 1 else ""
            return f"Error: {error_name} {additional_info}"
        except ValueError:
            return f"Unknown error code: {error_code}, data: {data[1:]}"
    return f"Invalid error data: {data}"

def parse_comms_warning(data):
    """Parse warning data based on CommsWarnings enum"""
    if len(data) >= 1:
        warning_code = data[0]
        try:
            warning_name = CommsWarnings(warning_code).name
            additional_info = f"Additional data: {data[1:]}" if len(data) > 1 else ""
            return f"Warning: {warning_name} {additional_info}"
        except ValueError:
            return f"Unknown warning code: {warning_code}, data: {data[1:]}"
    return f"Invalid warning data: {data}"

def parse_debug_message(data):
    """Parse debug message data, attempting to decode as text"""
    try:
        return f"Debug: {data.decode('utf-8')}"
    except UnicodeDecodeError:
        return f"Debug (binary): {data.hex()}"

def parse_icm29048_data(data):
    """
    Parse IMU data from ICM20948
    
    Data format:
    - Byte 0: IMU ID (1 or 2)
    - Bytes 1-6: Accelerometer data (three int16_t values X, Y, Z)
    - Bytes 7-12: Gyroscope data (three int16_t values X, Y, Z)
    """
    if len(data) < 13:
        return f"Invalid ICM20948 data (too short): {data.hex()}"
    
    try:
        # Extract IMU ID
        imu_id = data[0]
        
        # Extract accelerometer data (3 int16_t values)
        accel_x = struct.unpack("<h", data[1:3])[0]
        accel_y = struct.unpack("<h", data[3:5])[0]
        accel_z = struct.unpack("<h", data[5:7])[0]
        
        # Extract gyroscope data (3 int16_t values)
        gyro_x = struct.unpack("<h", data[7:9])[0]
        gyro_y = struct.unpack("<h", data[9:11])[0]
        gyro_z = struct.unpack("<h", data[11:13])[0]
        
        # Convert raw values to physical units (optional)
        # This depends on your IMU configuration and sensitivity settings
        # These are example conversions, you may need to adjust based on your settings
        accel_scale = 1/16384.0  # For ±2g range (adjust for your config)
        gyro_scale = 1/131.0     # For ±250°/s range (adjust for your config)
        
        accel_x_g = accel_x * accel_scale
        accel_y_g = accel_y * accel_scale
        accel_z_g = accel_z * accel_scale
        
        gyro_x_dps = gyro_x * gyro_scale
        gyro_y_dps = gyro_y * gyro_scale
        gyro_z_dps = gyro_z * gyro_scale
        
        # Format the output
        return (f"IMU #{imu_id} Data:\n"
                f"  Accelerometer (raw): X={accel_x}, Y={accel_y}, Z={accel_z}\n"
                f"  Accelerometer (g): X={accel_x_g:.3f}, Y={accel_y_g:.3f}, Z={accel_z_g:.3f}\n"
                f"  Gyroscope (raw): X={gyro_x}, Y={gyro_y}, Z={gyro_z}\n"
                f"  Gyroscope (°/s): X={gyro_x_dps:.3f}, Y={gyro_y_dps:.3f}, Z={gyro_z_dps:.3f}")
        
    except Exception as e:
        return f"Error parsing ICM20948 data: {e}, data: {data.hex()}"

def main():
    parser = argparse.ArgumentParser(description='USB Serial Data Receiver')
    parser.add_argument('--port', default='/dev/ttyACM0', help='Serial port to use')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate')
    parser.add_argument('--log-file', help='Log file path (default: logs/usb_YYYY-MM-DD_HH-MM-SS.log)')
    parser.add_argument('--log-level', default='INFO', 
                        choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
                        help='Logging level')
    args = parser.parse_args()
    
    # Create logs directory if it doesn't exist
    os.makedirs('logs', exist_ok=True)
    
    # Generate default log filename with timestamp if not provided
    if not args.log_file:
        timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        args.log_file = f"logs/usb_{timestamp}.log"
    
    # Setup logger
    log_level = getattr(logging, args.log_level)
    logger = setup_logger(args.log_file, log_level)
    
    logger.info(f"USB Serial Data Receiver started")
    logger.info(f"Opening {args.port} at {args.baud} baud...")
    logger.info(f"Logging to {args.log_file} at level {args.log_level}")
    
    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
        logger.info(f"Connected to {args.port}")
        
        # Clear any initial data
        ser.reset_input_buffer()
        
        # Protocol parsing state
        state = "LENGTH"
        expected_length = 0
        data_buffer = bytearray()
        
        while True:
            try:
                if state == "LENGTH":
                    # Read the length (2 bytes, little endian)
                    length_bytes = ser.read(2)
                    if len(length_bytes) == 2:
                        expected_length = struct.unpack("<H", length_bytes)[0]
                        state = "DATA"
                        data_buffer = bytearray()
                        logger.debug(f"Expecting {expected_length} bytes of data")
                
                elif state == "DATA":
                    # Read the expected data (identifier + payload)
                    if ser.in_waiting >= expected_length:
                        data = ser.read(expected_length)
                        if len(data) == expected_length:
                            # First byte is the identifier
                            identifier = data[0]
                            payload = data[1:]
                            
                            try:
                                id_name = SendIdentifiers(identifier).name
                                logger.info(f"Received packet with identifier: {id_name} ({identifier})")
                                
                                # Process based on identifier
                                if identifier == SendIdentifiers.COMMS_WARN:
                                    message = parse_comms_warning(payload)
                                    logger.warning(message)
                                elif identifier == SendIdentifiers.COMMS_ERROR:
                                    message = parse_comms_error(payload)
                                    logger.error(message)
                                elif identifier == SendIdentifiers.COMMS_DEBUG:
                                    message = parse_debug_message(payload)
                                    logger.debug(message)
                                elif identifier == SendIdentifiers.SPI_FAIL:
                                    message = f"SPI failure: {payload.hex()}"
                                    logger.error(message)
                                elif identifier == SendIdentifiers.LED_LISTENER_FAIL:
                                    message = f"LED listener failure: {payload.hex()}"
                                    logger.error(message)
                                elif identifier == SendIdentifiers.ICM29048:
                                    message = parse_icm29048_data(payload)
                                    logger.info(message)
                                else:
                                    message = f"Unknown identifier: {identifier}, Raw data: {payload.hex()}"
                                    logger.warning(message)
                                
                            except ValueError:
                                message = f"Unknown identifier: {identifier}, Raw data: {payload.hex()}"
                                logger.warning(message)
                            
                            state = "LENGTH"  # Back to reading length
                
                # Small delay to prevent CPU hogging
                time.sleep(0.001)
                
            except KeyboardInterrupt:
                logger.info("Exiting due to keyboard interrupt")
                break
            except Exception as e:
                logger.exception(f"Error during processing: {e}")
                state = "LENGTH"  # Reset state on error
                time.sleep(0.5)
                
    except serial.SerialException as e:
        logger.error(f"Error opening serial port: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            logger.info("Serial port closed")
        logger.info("USB Serial Data Receiver terminated")

if __name__ == "__main__":
    main()