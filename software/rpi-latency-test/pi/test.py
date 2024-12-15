import serial
import time
import os
import termios
import fcntl
import random

def configure_ultra_low_latency(ser):
    fd = ser.fileno()

    # Get current attributes
    attrs = termios.tcgetattr(fd)

    # Input modes - clear processing flags
    attrs[0] &= ~(termios.IGNBRK | termios.BRKINT | termios.PARMRK | 
        termios.ISTRIP | termios.INLCR | termios.IGNCR | 
        termios.ICRNL | termios.IXON)

    # Output modes - clear processing
    attrs[1] &= ~termios.OPOST

    # Control modes
    attrs[2] &= ~(termios.HUPCL)

    # Local modes - clear canonical, echo, signals
    attrs[3] &= ~(termios.ECHO | termios.ECHONL | termios.ICANON | 
        termios.ISIG | termios.IEXTEN)

    # Control characters
    attrs[6][termios.VMIN] = 1   # Minimum number of characters to read
    attrs[6][termios.VTIME] = 0  # No timeout

    # Set modified attributes
    termios.tcsetattr(fd, termios.TCSANOW, attrs)

    # Additional low-level configuration
    import fcntl
    try:
        # Disable blocking on read
        fl = fcntl.fcntl(fd, fcntl.F_GETFL)
        fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
    except Exception as e:
        print(f"Could not set non-blocking mode: {e}")

def latency_test(port='/dev/ttyACM0', baudrate=115200, iterations=1000):
    with serial.Serial(
        port=port, 
        baudrate=baudrate, 
        timeout=0,  # Non-blocking
        write_timeout=0
    ) as ser:
        # Configure for low latency
        configure_ultra_low_latency(ser)

        # Small delay to stabilize
        time.sleep(0.1)

        latencies = []

        for _ in range(iterations):
            test_byte = random.randint(0, 255)
            random_delay_nanoseconds = random.randint(0, 10000000)
            # High-precision timing
            start = time.perf_counter_ns()

            # Send test byte
            ser.write(bytes([test_byte]))
            ser.flush()

            # Wait for echo
            while ser.in_waiting == 0:
                pass

            # Read echo
            echo = ser.readall()[0]
            print(echo)
            print(test_byte)
            if echo != test_byte:
                print("wrong echo result!")

            # Calculate latency
            end = time.perf_counter_ns()

            if echo == test_byte:
                latency = end - start
                print(f"latency: {latency/1000000}")
                latencies.append(latency)

            # delay for random_delay_nanoseconds
            while (time.perf_counter_ns() - end < random_delay_nanoseconds):
                continue

        # Print statistics
        import statistics
        print("Latency Statistics (nanoseconds):")
        print(f"Mean: {statistics.mean(latencies)}")
        print(f"Median: {statistics.median(latencies)}")
        print(f"Min: {min(latencies)}")
        print(f"Max: {max(latencies)}")
        print(f"Standard Deviation: {statistics.stdev(latencies)}")

# Run the test
latency_test(iterations=1000, baudrate=8000000)
