import serial
import time
import sys
import select
import struct

# Configure the serial port - adjust as needed
SERIAL_PORT = "/dev/ttyACM0"  # Change to your device
BAUD_RATE = 115200  # Change to match your device


def send_blink_command(ser, duration_ms):
    """
    Send a command to blink an LED for the specified duration

    Command format:
    - 2 bytes: length of remaining data (3 bytes) in little endian
    - 1 byte: identifier (255)
    - 2 bytes: blink duration in ms (little endian uint16)
    """
    # Pack the duration as a 2-byte little endian value (uint16)
    duration_bytes = struct.pack("<H", duration_ms)

    # Create the command with adjusted length (3 bytes total remaining)
    command = (
        bytearray(
            [
                0x03,
                0x00,  # Length (3) in little endian
                0xFF,  # Identifier (255)
            ]
        )
        + duration_bytes
    )

    # Send the command
    print(f"Sending blink command for {duration_ms} ms")
    print("Bytes:", " ".join(f"{b:02X}" for b in command))
    ser.write(command)


def main():
    try:
        # Open serial port
        ser = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.1,
        )

        print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud")

        print("\nLED Blink Controller")
        print("--------------------")
        print("Commands:")
        print("  <number> : Send blink command for specified milliseconds (0-65535)")
        print("  q        : Quit the program")
        print("  h        : Show this help menu")

        while True:
            # Print prompt and get user input
            sys.stdout.write("\n> ")
            sys.stdout.flush()

            # Non-blocking check for serial data while waiting for user input
            while not select.select([sys.stdin], [], [], 0)[0]:
                if ser.in_waiting > 0:
                    data = ser.read(ser.in_waiting)
                    hex_data = " ".join(f"{b:02X}" for b in data)
                    print(f"\rReceived: {hex_data}")
                    try:
                        ascii_data = data.decode("ascii", errors="replace")
                        print(f"ASCII: {ascii_data}")
                    except:
                        pass
                    sys.stdout.write("> ")
                    sys.stdout.flush()
                time.sleep(0.1)

            # Get and process user input
            user_input = sys.stdin.readline().strip()

            if user_input.lower() == "q":
                print("Exiting...")
                break
            elif user_input.lower() == "h":
                print("\nLED Blink Controller")
                print("--------------------")
                print("Commands:")
                print(
                    "  <number> : Send blink command for specified milliseconds (0-65535)"
                )
                print("  q        : Quit the program")
                print("  h        : Show this help menu")
            else:
                try:
                    # Try to convert input to an integer
                    duration_ms = int(user_input)
                    if duration_ms < 0:
                        print("Duration must be positive")
                    elif duration_ms > 65535:
                        print("Duration must be at most 65535 (uint16 max)")
                    else:
                        send_blink_command(ser, duration_ms)
                except ValueError:
                    print(
                        f"Invalid input: '{user_input}'. Enter a number for blink duration or 'q' to quit."
                    )

    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        if "ser" in locals() and ser.is_open:
            ser.close()
            print("Serial port closed")


if __name__ == "__main__":
    main()

