#pragma once

/**
 * NOTE:
 * Uses the pico_stdio_usb drivers to handle usb-cdc communication while preserving the usb boot interface.
 * This is not ideal, as the drivers don't expose lower level APIs.
 * It would be more performant and maybe lower latency to reimplement this ourselves.
 * Not urgent though, probably does not matter by too much.
 */

#include "class/cdc/cdc.h"
#include "comms/default_usb_config.h"
#include "identifiers.hpp"
#include "osal/osal_freertos.h"
#include "types.hpp"
extern "C" {
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <tusb.h>
}

/**
 * WARNING: RP2040 and RPi are all little endian (least significant byte first)
 * INFO:
 * Communication format is defined as such, in both directions:
 * Byte 1 & 2: Length (least significant byte first) (excludes length bytes)
 * (pico will check that length <= MAX_RX_BUF_SIZE)
 * WARNING: LENGTH INCLUDES IDENTIFIER BYTE. This is done for convenience in the recieving state.
 * INFO:
 * Byte 3: Identifier (u8 enum)
 * The rest is passed to the specific handler.
 */
namespace usb {

static const types::u32 HOST_CONNECTION_TIMEOUT =
    1000 * 1000;                                              // in microseconds
static const types::u32 CDC_CONNECTION_TIMEOUT = 1000 * 1000; // in microseconds

static const types::u16 MAX_RX_BUF_SIZE = USB_RX_BUFSIZE;
static const types::u16 MAX_TX_BUF_SIZE = USB_TX_BUFSIZE;

static const types::u8 N_LENGTH_BYTES = 2;
static const types::u16 MAX_RX_PACKET_LENGTH = MAX_RX_BUF_SIZE;
static const types::u16 MAX_TX_PACKET_LENGTH = MAX_TX_BUF_SIZE;

static const types::u8 TUSB_INIT_SLEEP_LOOP_TIME = 100; // in microseconds

using CDCLineCodingCB = void (*)(types::u8, const cdc_line_coding_t *, void *);
using VendorControlXferCB = bool (*)(types::u8, types::u8,
                                     const tusb_control_request_t *, void *);
using CDCRxCB = void (*)(types::u8, void *);

extern CDCLineCodingCB CDC_line_coding_cb_fn;
extern void *CDC_line_coding_cb_user_args;

extern VendorControlXferCB vendor_control_xfer_cb_fn;
extern void *vendor_control_xfer_cb_user_args;

extern CDCRxCB CDC_rx_cb_fn;
extern void *CDC_rx_cb_user_args;

struct CurrentRXState {
  bool length_bytes_recieved = false;
  types::u16 expected_length = 0;
  bool recieved = false;
  types::u8 *data_buffer = nullptr;
  inline void reset(void) {
    length_bytes_recieved = false;
    expected_length = 0;
    recieved = false;
    memset(data_buffer, 0, MAX_RX_BUF_SIZE);
  }
};

class CDC {
public:
  CDC();
  /**
   * @brief initialises USB CDC
   * @warning must be callled after rtos scheduler is started, if using with rtos (due to tinyusb)
   * @returns true if successfully initialised, false if not
   */
  bool init(void);

  inline bool initialised(void) { return _initialised; }

  /**
   * @brief blocks until host is connected through USB.
   * @param timeout: timeout in microseconds, defaults to HOST_CONNECTION_TIMEOUT
   * @returns true if host connection achieved, false if not
   */
  bool wait_for_host_connection(types::u32 timeout = HOST_CONNECTION_TIMEOUT);

  /**
   * @brief blocks until CDC connection to host is established.
   * @param timeout: timeout in microseconds, defaults to CDC_CONNECTION_TIMEOUT
   * @returns true if host connection achieved, false if not
   */
  bool wait_for_CDC_connection(types::u32 timeout = CDC_CONNECTION_TIMEOUT);

  /**
   * @brief flush write buffer
   */
  static void flush(void);

  /**
   * @brief writes data, formatted correctly, will flush buffer.
   * @param identifier: identifier for the sent data
   * @param data: u8 array
   * @param data_len: length of data
   * @returns true if successfully sent, false if not
   */
  static bool write(const comms::SendIdentifiers identifier,
                    const types::u8 *data, const types::u16 data_len);

  /**
   * @brief adds data, formatted correctly, to an internal buffer, need to call interrupt_data_flush after all writes in an interrupt is done
   * @brief WARN: NOT lossless! any interrupts that send data should only send data that is ok to be lost
   * @brief WARN: data will be lost if not flushed before buffer (of size MAX_TX_BUF_SIZE)
   * @brief WARN: NOT thread safe! all interrupts that send data should only be on one core
   * @param identifier: identifier for the sent data
   * @param data: u8 array
   * @param data_len: length of data
   * @returns true if successfully written to buffer, false if not
   */
  static bool interrupt_write(const comms::SendIdentifiers identifier,
                              const types::u8 *data, const types::u16 data_len);

  /**
   * @brief sets event flag that triggers event to process the interrupt buffer.
   * @brief NOTE: this MUST be called at the end of a sequence of any interrupt_write() calls.
   */
  static void interrupt_flush(void);

  /**
   * @brief read data into buffer with some max length
   * @param data: u8 array
   * @param len: length to be read
   * @param timeout: max time spent in this function, in millis
   * @note does NOT care about null terminator!
   * @note this function should not really be used
   * @returns how many characters read
   */
  static types::u32 read(types::u8 *data, types::u32 len, types::u32 timeout);

private:
  /**
   * @brief callback that adds to data buffer while parsing length. Feeds command into command_recv_callback.
   * @param interface: id of tusb cdc interface (probably wont use)
   * @param args: ptr to CurrentRXState (is cast to void ptr for flexibility)
   */
  static void _rx_cb(types::u8 interface, void *args);

  /**
   * @brief callback to run on change of line coding settings from host.
   * @brief used mainly for baud rate reset to bootloader hack
   * @param interface: id of tusb cdc interface (probably wont use)
   * @param coding: tusb cdc line coding settings
   * @param args: custom args to pass into handler
   */
  static void _line_coding_cb(types::u8 interface,
                              cdc_line_coding_t const *coding, void *args);

  /**
   * @brief callback to run on vendor control from host
   * @brief used for picotool reset and reset to bootloader
   * @brief not very sure how this works so not documenting the parameters
   * @param args: custom args to pass into handler
   */
  static bool _vendor_control_xfer_cb(types::u8 rhport, types::u8 stage,
                                      tusb_control_request_t const *request,
                                      void *args);

  inline static void _tud_task_caller(void *args) {
    for (;;) {
      tud_task();
    }
  }

  bool _initialised = false;
  bool _host_connected = false;
  bool _cdc_connected = false;

  types::u8 _read_buffer[MAX_RX_BUF_SIZE] = {0};
  types::u8 _cmd_buffer[MAX_RX_BUF_SIZE] = {0};
  types::u8 _interrupt_write_buffer[MAX_TX_BUF_SIZE] = {0};

  // hooks for commands

  TaskHandle_t *tud_task_handle = nullptr;
  static CurrentRXState _current_rx_state;
};

} // namespace usb
