#include "comms/usb.hpp"
#include "comms/errors.hpp"
#include "comms/identifiers.hpp"
#include "types.hpp"
#include <cstring>
#include <cstdarg>
#include <iostream>
#include <chrono>
#include <thread>
#include <sys/select.h>
#include <errno.h>

using namespace types;

namespace usb {

CDC::CDC() 
  : fd_(-1), initialized_(false), connected_(false), running_(false) {
  current_rx_state_.data_buffer = read_buffer_;
  current_rx_state_.reset();
}

CDC::~CDC() {
  running_ = false;
  if (read_thread_.joinable()) {
    read_thread_.join();
  }
  
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

bool CDC::init(const std::string& device_path) {
  device_path_ = device_path;
  
  // Open serial port
  fd_ = open(device_path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0) {
    std::cerr << "Failed to open " << device_path << ": " << strerror(errno) << std::endl;
    return false;
  }
  
  // Configure termios
  struct termios tty;
  if (tcgetattr(fd_, &tty) != 0) {
    std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
    close(fd_);
    fd_ = -1;
    return false;
  }
  
  // Clear parity bit, disabling parity
  tty.c_cflag &= ~PARENB;
  // Clear stop field, only one stop bit used
  tty.c_cflag &= ~CSTOPB;
  // Clear size field, 8 bits per byte
  tty.c_cflag &= ~CSIZE;
  // Set size to 8 bits per byte
  tty.c_cflag |= CS8;
  // Disable RTS/CTS hardware flow control
  tty.c_cflag &= ~CRTSCTS;
  // Turn on READ & ignore ctrl lines
  tty.c_cflag |= CREAD | CLOCAL;
  
  // Disable canonical mode
  tty.c_lflag &= ~ICANON;
  // Disable echo
  tty.c_lflag &= ~ECHO;
  // Disable erasure
  tty.c_lflag &= ~ECHOE;
  // Disable new-line echo
  tty.c_lflag &= ~ECHONL;
  // Disable interpretation of INTR, QUIT and SUSP
  tty.c_lflag &= ~ISIG;
  
  // Disable software flow control
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  // Disable special handling of received bytes
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
  
  // Disable special handling of output bytes
  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;
  
  // Set baud rate (115200)
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);
  
  // Apply settings
  if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
    std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
    close(fd_);
    fd_ = -1;
    return false;
  }
  
  // Make port blocking again
  fcntl(fd_, F_SETFL, 0);
  
  // Set initialized and start read thread
  initialized_ = true;
  running_ = true;
  read_thread_ = std::thread(&CDC::read_thread_func, this);
  
  connected_ = true;
  return true;
}

bool CDC::wait_for_device_connection(u32 timeout) {
  if (!initialized_) {
    return false;
  }
  
  auto start_time = std::chrono::steady_clock::now();
  while (!connected_) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    
    if (elapsed > timeout) {
      return false;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  
  return true;
}

bool CDC::device_connected() const {
  return connected_;
}

bool CDC::is_initialized() const {
  return initialized_;
}

void CDC::flush() {
  if (!connected_) {
    return;
  }
  
  // Nothing to do - we're already writing directly in the write function
}

bool CDC::write(const comms::SendIdentifiers identifier, const u8 *data, const u16 data_len) {
  if (!connected_) {
    return false;
  }
  
  std::lock_guard<std::mutex> lock(write_mutex_);
  
  u16 reported_len = data_len + sizeof(identifier);
  u16 packet_len = sizeof(reported_len) + reported_len;
  
  if (packet_len > MAX_TX_BUF_SIZE) {
    return false;
  }
  
  // Create buffer for entire message
  std::vector<u8> buffer(packet_len);
  
  // Copy length
  std::memcpy(buffer.data(), &reported_len, sizeof(reported_len));
  
  // Copy identifier
  buffer[sizeof(reported_len)] = static_cast<u8>(identifier);
  
  // Copy data
  if (data_len > 0) {
    std::memcpy(buffer.data() + sizeof(reported_len) + sizeof(identifier), 
                data, data_len);
  }
  
  // Write to serial port
  size_t bytes_written = 0;
  while (bytes_written < buffer.size()) {
    ssize_t result = ::write(fd_, buffer.data() + bytes_written, buffer.size() - bytes_written);
    if (result < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Would block, try again after a short delay
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }
      return false; // Error
    }
    bytes_written += result;
  }
  
  // Ensure data is sent
  tcdrain(fd_);
  return true;
}

bool CDC::printf(const char *format, ...) {
  if (!connected_) {
    return false;
  }
  
  char formatted[MAX_TX_BUF_SIZE];
  va_list args;
  va_start(args, format);
  int size = vsnprintf(formatted, sizeof(formatted), format, args);
  va_end(args);
  
  if (size < 0) {
    return false;
  }
  
  std::lock_guard<std::mutex> lock(write_mutex_);
  ssize_t res = ::write(fd_, formatted, size);
  return (res == size);
}

bool CDC::attach_listener(comms::RecvIdentifiers identifier, 
                          std::function<void(const types::u8*, types::u16)> callback) {
  if (!callback) {
    return false;
  }
  
  std::lock_guard<std::mutex> lock(listeners_mutex_);
  command_listeners_[static_cast<u8>(identifier)] = callback;
  return true;
}

void CDC::read_thread_func() {
  while (running_) {
    if (!connected_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    
    // Use select to wait for data with timeout
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd_, &read_fds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000; // 100ms timeout
    
    int select_result = select(fd_ + 1, &read_fds, nullptr, nullptr, &timeout);
    
    if (select_result < 0) {
      // Error occurred
      if (errno != EINTR) {
        connected_ = false;
      }
      continue;
    } else if (select_result == 0) {
      // Timeout occurred, no data
      continue;
    }
    
    // Data is available, process it
    process_rx_data();
  }
}

void CDC::process_rx_data() {
  // State machine for processing packets
  CurrentRXState &state = current_rx_state_;
  
  while (running_ && connected_) {
    if (!state.length_bytes_received) {
      // Need to read the length bytes first
      u8 length_buf[N_LENGTH_BYTES];
      ssize_t bytes_read = read(fd_, length_buf, N_LENGTH_BYTES);
      
      if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          // No data available right now
          return;
        }
        // Error occurred
        connected_ = false;
        return;
      } else if (bytes_read == 0) {
        // EOF - device disconnected
        connected_ = false;
        return;
      } else if (bytes_read < N_LENGTH_BYTES) {
        // Incomplete read, wait for more data
        return;
      }
      
      // Successfully read length bytes
      state.expected_length = *reinterpret_cast<u16*>(length_buf);
      state.length_bytes_received = true;
      
      if (state.expected_length > MAX_RX_BUF_SIZE) {
        // Length exceeds buffer size
        comms::CommsErrors err = comms::CommsErrors::PACKET_RECV_OVER_MAX_BUFSIZE;
        write(comms::SendIdentifiers::COMMS_ERROR, reinterpret_cast<u8*>(&err), sizeof(err));
        state.reset();
        return;
      }
    } else {
      // Length bytes received, now read the data
      ssize_t bytes_read = read(fd_, state.data_buffer, state.expected_length);
      
      if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          // No data available right now
          return;
        }
        // Error occurred
        connected_ = false;
        return;
      } else if (bytes_read == 0) {
        // EOF - device disconnected
        connected_ = false;
        return;
      } else if (bytes_read < state.expected_length) {
        // Incomplete read, wait for more data
        return;
      }
      
      // Successfully read data bytes
      u8 identifier = state.data_buffer[0];
      
      // Call appropriate listener
      {
        std::lock_guard<std::mutex> lock(listeners_mutex_);
        auto it = command_listeners_.find(identifier);
        if (it != command_listeners_.end()) {
          // Call the callback with data (skipping the identifier byte)
          it->second(state.data_buffer + 1, state.expected_length - 1);
        } else {
          // No handler for this identifier
          comms::CommsErrors err = comms::CommsErrors::CALLING_UNATTACHED_LISTENER;
          u8 msg[] = {static_cast<u8>(err), identifier};
          write(comms::SendIdentifiers::COMMS_ERROR, msg, sizeof(msg));
        }
      }
      
      // Reset state for next packet
      state.reset();
    }
  }
}

} // namespace usb