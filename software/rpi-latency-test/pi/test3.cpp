#include <iostream>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <linux/serial.h> // Required for low latency configuration
#include <sys/ioctl.h>

#define SERIAL_PORT "/dev/ttyACM0" // Adjust as needed
#define BAUD_RATE 921600
#define TEST_DATA "Ping"
#define BUFFER_SIZE 64

void setLowLatencyMode(int fd) {
    struct serial_struct serial;
    if (ioctl(fd, TIOCGSERIAL, &serial) < 0) {
        std::cerr << "Error: Unable to get serial settings for low latency mode." << std::endl;
        return;
    }

    serial.flags |= ASYNC_LOW_LATENCY; // Enable low latency mode
    if (ioctl(fd, TIOCSSERIAL, &serial) < 0) {
        std::cerr << "Error: Unable to set low latency mode." << std::endl;
    } else {
        std::cout << "Low latency mode enabled." << std::endl;
    }
}

int main() {
    // Open the serial port
    int serial_fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0) {
        std::cerr << "Error: Unable to open serial port " << SERIAL_PORT << std::endl;
        return 1;
    }

    // Configure low latency mode
    setLowLatencyMode(serial_fd);

    // Configure the serial port
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(serial_fd, &tty) != 0) {
        std::cerr << "Error: Unable to get terminal attributes" << std::endl;
        close(serial_fd);
        return 1;
    }

    // Set baud rate, character size, and disable unnecessary options
    cfsetospeed(&tty, BAUD_RATE);
    cfsetispeed(&tty, BAUD_RATE);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                     // Disable break processing
    tty.c_lflag = 0;                            // No signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                            // No remapping, no delays
    tty.c_cc[VMIN] = 1;                         // Read at least one byte
    tty.c_cc[VTIME] = 1;                        // Timeout in deciseconds

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error: Unable to set terminal attributes" << std::endl;
        close(serial_fd);
        return 1;
    }

    // Buffers
    char tx_buffer[] = TEST_DATA;
    char rx_buffer[BUFFER_SIZE] = {0};

    // Latency test loop
    for (int i = 0; i < 10; ++i) {
        // Record start time
        auto start = std::chrono::high_resolution_clock::now();

        // Write data to the serial port
        int bytes_written = write(serial_fd, tx_buffer, strlen(tx_buffer));
        if (bytes_written < 0) {
            std::cerr << "Error: Unable to write to serial port" << std::endl;
            break;
        }

        // Read the echoed data from the serial port
        int bytes_read = read(serial_fd, rx_buffer, bytes_written);
        if (bytes_read < 0) {
            std::cerr << "Error: Unable to read from serial port" << std::endl;
            break;
        }

        // Record end time
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        // Validate response
        if (strncmp(tx_buffer, rx_buffer, bytes_written) == 0) {
            std::cout << "Latency: " << elapsed.count() << " ms" << std::endl;
        } else {
            std::cerr << "Error: Data mismatch" << std::endl;
        }

        // Clear the read buffer for the next iteration
        memset(rx_buffer, 0, BUFFER_SIZE);
        usleep(100000); // 100 ms delay between tests
    }

    // Close the serial port
    close(serial_fd);
    return 0;
}
