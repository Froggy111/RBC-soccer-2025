#pragma once

#include "comms/default_usb_config.h"
#include "identifiers.hpp"
#include "types.hpp"
extern "C" {
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>
#include <task.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <tusb.h>
#include <class/cdc/cdc.h>
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

/* ******** *
 * Settings *
 * ******** */

static const types::u32 HOST_CONNECTION_TIMEOUT = 1000; // in milliseconds
static const types::u32 CDC_CONNECTION_TIMEOUT = 1000;  // in milliseconds

static const types::u16 MAX_RX_BUF_SIZE = USB_RX_BUFSIZE;
static const types::u16 MAX_TX_BUF_SIZE = USB_TX_BUFSIZE;
static const types::u16 MAX_INTERRUPT_TX_BUF_SIZE = MAX_TX_BUF_SIZE * 4;

static const types::u8 N_LENGTH_BYTES = 2;
static const types::u16 MAX_RX_PACKET_LENGTH = MAX_RX_BUF_SIZE;
static const types::u16 MAX_TX_PACKET_LENGTH = MAX_TX_BUF_SIZE;

static const types::u16 TUSB_STATUS_CHECKING_TIME = 1; // in milliseconds

static const types::u8 HOST_CONNECTED_BIT = 0b00000001;
static const types::u8 CDC_CONNECTED_BIT = 0b00000010;

/* **************************************************************** *
 * TinyUSB callbacks (done this way for convenience and clean-ness) *
 * **************************************************************** */

using CDCLineCodingCB = void (*)(types::u8, const cdc_line_coding_t *, void *);
using VendorControlXferCB = bool (*)(types::u8, types::u8,
                                     const tusb_control_request_t *, void *);
using CDCRxCB = void (*)(types::u8, void *);
using MountCB = void (*)(void *);
using UnmountCB = void (*)(void *);
using CDCLineStateCB = void (*)(types::u8, bool, bool, void *);

extern CDCLineCodingCB CDC_line_coding_cb_fn;
extern void *CDC_line_coding_cb_user_args;

extern VendorControlXferCB vendor_control_xfer_cb_fn;
extern void *vendor_control_xfer_cb_user_args;

extern CDCRxCB CDC_rx_cb_fn;
extern void *CDC_rx_cb_user_args;

extern MountCB mount_cb_fn;
extern void *mount_cb_user_args;

extern UnmountCB unmount_cb_fn;
extern void *unmount_cb_user_args;

extern CDCLineStateCB CDC_line_state_cb_fn;
extern void *CDC_line_state_cb_user_args;

/* ******* *
 * Structs *
 * ******* */

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

/* ********** *
 * Main class *
 * ********** */

class CDC {
public:
  CDC();

  /**
   * @brief initialises all the variables and tasks needed.
   * @brief WARN: should be called before starting FreeRTOS scheduler.
   * @returns true if successfully initialised, false if not
   */
  bool init(void);

  inline bool initialised(void) { return tud_inited(); }

  /**
   * @brief blocks until host is connected through USB.
   * @param timeout: timeout in milliseconds, defaults to HOST_CONNECTION_TIMEOUT
   * @returns true if host connection achieved, false if not
   */
  bool wait_for_host_connection(types::u32 timeout = HOST_CONNECTION_TIMEOUT);

  /**
   * @brief blocks until CDC connection to host is established.
   * @param timeout: timeout in milliseconds, defaults to CDC_CONNECTION_TIMEOUT
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
   * @brief adds data, formatted correctly, to an internal buffer, need to call flush_from_IRQ after all writes in an interrupt is done
   * @brief WARN: NOT lossless! any interrupts that send data should only send data that is ok to be lost
   * @brief WARN: data will be lost if not flushed before buffer (of size MAX_TX_BUF_SIZE)
   * @param identifier: identifier for the sent data
   * @param data: u8 array
   * @param data_len: length of data
   * @param xHigherPriorityTaskWoken: needed flag for freertos, used in portYIELD_FROM_ISR at the end of an interrupt.
   * @returns true if successfully written to buffer, false if not
   */
  static bool write_from_IRQ(const comms::SendIdentifiers identifier,
                             const types::u8 *data, const types::u16 data_len,
                             BaseType_t *xHigherPriorityTaskWoken);

  /**
   * @brief sets event flag that triggers event to process the interrupt buffer.
   * @brief NOTE: this MUST be called at the end of a sequence of any interrupt_write() calls.
   * @param higher_priority_task_woken FreeRTOS flag for if a higher priority task is woken by a command. used for portYIELD_FROM_ISR
   */
  static void flush_from_IRQ(BaseType_t *higher_priority_task_woken);

  /**
   * @brief attach a task to listen to any incoming commands on USB
   * @param identifier: command identifier to listen to
   * @param handle: task handle. WARN: this must be initialised beforehand.
   * @param mutex: command buffer mutex. WARN: this must be initialised beforehand.
   * @param buffer: command bufffer. WARN: this must be initialised beforehand, and should be global/static.
   * @param length: max length for command buffer. WARN: this must be large enough to accomodate the largest possible command.
   * @returns whether the attachment was successful. Checks for nullptr. Will override, but will warn if there is an existing listener.
   */
  bool attach_listener(comms::RecvIdentifiers identifier, TaskHandle_t handle,
                       SemaphoreHandle_t mutex, types::u8 *buffer,
                       types::u16 length);

private:
  /* **************** *
  * Private functions *
  * ***************** */

  // helper for writing to _interrupt_write_buffer. does not do any checks!
  // just wraps memcpy and incrementing _interrupt_write_buffer_index
  inline static void _interrupt_write_buffer_write(const void *data,
                                                   size_t size) {
    memcpy(_interrupt_write_buffer + _interrupt_write_buffer_index, data, size);
    _interrupt_write_buffer_index += size;
  }
  static void _usb_device_task(void *args);

  static void _IRQ_write_flusher(void *args);

  /**
   * @brief callback that adds to data buffer while parsing length. Feeds command into command_recv_callback.
   * @brief this does not execute in an interrupt context.
   * @param interface: id of tusb cdc interface (probably wont use)
   * @param args: ptr to CurrentRXState (is cast to void ptr for flexibility)
   */
  static void _rx_cb(types::u8 interface, void *args);

  /**
   * @brief callback to run on change of line coding settings from host
   * @brief used mainly for baud rate reset to bootloader hack
   * @param interface: id of tusb cdc interface (probably wont use)
   * @param coding: tusb cdc line coding settings
   * @param args: custom args to pass into handler
   */
  static void _line_coding_cb(types::u8 interface,
                              cdc_line_coding_t const *coding, void *args);

  /**
   * @brief callback to run on change of line state settings from host
   * @brief used mainly for monitoring if CDC is connected
   * @param interface: id of tusb cdc interface (probably wont use)
   * @param dtr: data terminal ready
   * @param rts: request to send (used for hardware flow control, not necessary)
   * @param args: custom args to pass into handler
   */
  static void _line_state_cb(types::u8 interface, bool dtr, bool rts,
                             void *args);

  /**
   * @brief callback to run on vendor control from host
   * @brief used for picotool reset and reset to bootloader
   * @brief not very sure how this works so not documenting the parameters
   * @param args: custom args to pass into handler (unused)
   */
  static bool _vendor_control_xfer_cb(types::u8 rhport, types::u8 stage,
                                      tusb_control_request_t const *request,
                                      void *args);

  static void _mount_cb(void *args);
  static void _unmount_cb(void *args);

  /* *************************************************************** *
   * Private buffers, synchronisation primitives and other variables *
   * *************************************************************** */

  types::u8 _read_buffer[MAX_RX_BUF_SIZE] = {0};

  static SemaphoreHandle_t _write_mutex;

  // writing inside interrupts
  static types::u8 _interrupt_write_buffer[MAX_INTERRUPT_TX_BUF_SIZE];
  static types::u16 _interrupt_write_buffer_index;
  static SemaphoreHandle_t _interrupt_write_buffer_mutex;
  static TaskHandle_t _interrupt_write_task_handle;

  // hooks for other command handlers
  // these are indexed with the identifier
  static TaskHandle_t _command_task_handles[comms::identifier_arr_len];
  static SemaphoreHandle_t
      _command_task_buffer_mutexes[comms::identifier_arr_len];
  static types::u8 *_command_task_buffers[comms::identifier_arr_len];
  static types::u8 _command_task_buffer_lengths[comms::identifier_arr_len];

  // hooks for commands
  TaskHandle_t _tud_task_handle = nullptr;
  static CurrentRXState _current_rx_state;

  // tracking host connection and cdc connection
  static EventGroupHandle_t _connection_status_event;
};

} // namespace usb
