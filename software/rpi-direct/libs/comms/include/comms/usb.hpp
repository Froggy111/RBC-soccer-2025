#pragma once

#include "comms/default_usb_config.h"
#include "identifiers.hpp"
#include "types.hpp"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstring>
#include <string>
#include <functional>
#include <map>

/**
 * WARNING: RP2040 and RPi are all little endian (least significant byte first)
 * INFO:
 * Communication format is defined as such, in both directions:
 * Byte 1 & 2: Length (least significant byte first) (excludes length bytes)
 * (Raspberry Pi will check that length <= MAX_RX_BUF_SIZE)
 * WARNING: LENGTH INCLUDES IDENTIFIER BYTE. This is done for convenience in the receiving state.
 * INFO:
 * Byte 3: Identifier (u8 enum)
 * The rest is passed to the specific handler.
 */
namespace usb {

/* ******** *
 * Settings *
 * ******** */

static const types::u32 DEVICE_CONNECTION_TIMEOUT = 1000; // in milliseconds
static const types::u32 CDC_CONNECTION_TIMEOUT = 1000;  // in milliseconds

static const types::u16 MAX_RX_BUF_SIZE = USB_RX_BUFSIZE;
static const types::u16 MAX_TX_BUF_SIZE = USB_TX_BUFSIZE;
static const types::u16 MAX_INTERRUPT_TX_BUF_SIZE = MAX_TX_BUF_SIZE * 4;

static const types::u8 N_LENGTH_BYTES = 2;
static const types::u16 MAX_RX_PACKET_LENGTH = MAX_RX_BUF_SIZE;
static const types::u16 MAX_TX_PACKET_LENGTH = MAX_TX_BUF_SIZE;

/* ******* *
 * Structs *
 * ******* */

struct CurrentRXState {
  bool length_bytes_received = false;
  types::u16 expected_length = 0;
  types::u8 *data_buffer = nullptr;
  inline void reset(void) {
    length_bytes_received = false;
    expected_length = 0;
    memset(data_buffer, 0, MAX_RX_BUF_SIZE);
  }
};

/* ********** *
 * Main class *
 * ********** */

class CDC {
public:
  CDC();
  ~CDC();

  /**
   * @brief initializes all the variables and threads needed.
   * @param device path to serial device (e.g., "/dev/ttyACM0")
   * @returns true if successfully initialised, false if not
   */
  bool init(const std::string& device_path);

  /**
   * @brief blocks until device is connected through USB.
   * @param timeout timeout in milliseconds, defaults to DEVICE_CONNECTION_TIMEOUT
   * @returns true if device connection achieved, false if not
   */
  bool wait_for_device_connection(types::u32 timeout = DEVICE_CONNECTION_TIMEOUT);

  /**
   * @brief self-explanatory status checks
   */
  bool device_connected() const;
  bool is_initialized() const;

  /**
   * @brief flush write buffer
   */
  void flush();

  /**
   * @brief writes data, formatted correctly, will flush buffer.
   * @param identifier identifier for the sent data
   * @param data u8 array
   * @param data_len length of data
   * @returns true if successfully sent, false if not
   */
  bool write(const comms::SendIdentifiers identifier,
             const types::u8 *data, const types::u16 data_len);

  /**
   * @brief just a regular printf to the serial device
   * @returns false if device not connected before called
   */
  bool printf(const char *format, ...);

  /**
   * @brief attach a callback to listen to any incoming commands
   * @param identifier command identifier to listen to
   * @param callback function to call when command is received
   * @returns whether the attachment was successful
   */
  bool attach_listener(comms::RecvIdentifiers identifier, 
                      std::function<void(const types::u8*, types::u16)> callback);

private:
  /* **************** *
  * Private functions *
  * ***************** */
  void read_thread_func();
  void process_rx_data();

  /* *************************************************************** *
   * Private buffers, synchronisation primitives and other variables *
   * *************************************************************** */
  int fd_; // File descriptor for the serial port
  std::string device_path_;
  std::atomic<bool> initialized_;
  std::atomic<bool> connected_;
  std::atomic<bool> running_;

  std::thread read_thread_;
  std::mutex write_mutex_;
  
  types::u8 read_buffer_[MAX_RX_BUF_SIZE];
  CurrentRXState current_rx_state_;

  std::map<types::u8, std::function<void(const types::u8*, types::u16)>> command_listeners_;
  std::mutex listeners_mutex_;
};

} // namespace usb