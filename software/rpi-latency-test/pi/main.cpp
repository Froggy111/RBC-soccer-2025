#include <cstdlib>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

int main (int argc, char** argv) {
  if (argc != 3) {
    std::cout << "Invalid number of arguments. Expected arguments: serial interface (string), baud rate (int)" <<  std::endl;
  }
  const std::string chosen_serial_port = argv[1];
  int baud_rate = std::stoi(argv[2]);
  int serial_port = open(chosen_serial_port.c_str(), O_RDWR | O_NOCTTY);
  if (serial_port < 0) {
    std::cout << std::format("Error {} from open {}\n", errno, strerror(errno)) << std::endl;
    exit(EXIT_FAILURE);
  }

  struct termios tty;
  if(tcgetattr(serial_port, &tty) != 0) {
    std::cout << std::format("Error {} from tcgetattr: {}\n", errno, strerror(errno)) << std::endl;
    exit(EXIT_FAILURE);
  }

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;
  tty.c_cflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ECHOE;
  tty.c_lflag &= ~ECHONL;
  tty.c_lflag &= ~ISIG;
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;
  tty.c_cc[VTIME] = 232;
  tty.c_cc[VMIN] = 0;

  cfsetispeed(&tty, baud_rate);

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    std::cout << std::format("Error {} from tcsetattr: {}", errno, strerror(errno)) << std::endl;
  }

  std::cout << "entering loop" << std::endl;
  while (true) {
    char read_buf[128] = {0};
    int read_num = 1;
    bool continue_flag = false;

    // wait for ping
    do {
      std::cout << std::format("trying to read from serial port {}", chosen_serial_port) << std::endl;
      read_num = read(serial_port, &read_buf, sizeof(read_buf));
      std::cout << std::format("read num: {}", read_num) << std::endl;
      if (read_num == 0) {
        std::cout << std::format("Read of serial port {} timed out.", chosen_serial_port) << std::endl;
      }
      else if (read_num < 0) {
        std::cout << std::format("Read of serial port {} failed.", chosen_serial_port) << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    while (read_num <= 0);
    // std::cout << "recieved ping from pico" << std::endl;

    // check ping
    if (read_buf[0] != '1') {
      continue_flag = true;
    }
    if (continue_flag) {
      continue;
    }

    // send ping back
    char msg[] = "0";
    write(serial_port, msg, sizeof(msg));
    std::cout << "sent ping to pico" << std::endl;

    // recieve time data
    memset(read_buf, 0, sizeof(read_buf));
    do {
      read_num = read(serial_port, &read_buf, sizeof(read_buf));
      if (read_num == 0) {
        std::cout << std::format("Read of serial port {} timed out.", chosen_serial_port) << std::endl;
      }
      else if (read_num < 0) {
        std::cout << std::format("Read of serial port {} failed.", chosen_serial_port) << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    while (read_num <= 0);
    std::cout << std::format("returned time data from pico:\n{}", read_buf) << std::endl;
  }
}
