#include "comms/usb.hpp"
#include "comms/errors.hpp"
#include "comms/identifiers.hpp"
#include "types.hpp"
extern "C" {
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>
#include <task.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <tusb.h>
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
}

using namespace types;

namespace usb {

SemaphoreHandle_t CDC::_write_mutex = nullptr;

types::u8 CDC::_interrupt_write_buffer[MAX_INTERRUPT_TX_BUF_SIZE] = {0};
types::u16 CDC::_interrupt_write_buffer_index = 0;
SemaphoreHandle_t CDC::_interrupt_write_buffer_mutex = nullptr;
TaskHandle_t CDC::_interrupt_write_task_handle = nullptr;

// hooks for other command handlers
// these are indexed with the identifier
TaskHandle_t CDC::_command_task_handles[comms::identifier_arr_len] = {nullptr};
SemaphoreHandle_t CDC::_command_task_buffer_mutexes[comms::identifier_arr_len] =
    {nullptr};
types::u8 *CDC::_command_task_buffers[comms::identifier_arr_len] = {nullptr};
types::u8 CDC::_command_task_buffer_lengths[comms::identifier_arr_len] = {0};

CurrentRXState CDC::_current_rx_state = {};

EventGroupHandle_t CDC::_tusb_state_eventgroup = nullptr;

CDC::CDC() {
  // mostly stuff that should be set up before initialising tinyusb
  // set tusb callback handlers
  usb::vendor_control_xfer_cb_fn = _vendor_control_xfer_cb;
  usb::vendor_control_xfer_cb_user_args = nullptr;
  usb::CDC_line_coding_cb_fn = _line_coding_cb;
  usb::CDC_line_coding_cb_user_args = nullptr;
  usb::CDC_rx_cb_fn = _rx_cb;
  usb::CDC_rx_cb_user_args = &_current_rx_state;
  usb::CDC_line_state_cb_fn = _line_state_cb;
  usb::mount_cb_fn = _mount_cb;
  usb::unmount_cb_fn = _unmount_cb;

  // setup buffers
  _current_rx_state.data_buffer = _read_buffer;
  _current_rx_state.reset();
}

bool CDC::init(void) {
  // if (!tusb_init()) {
  //   return false; // some error, idk
  // }
  // create sync primitives
  CDC::_write_mutex = xSemaphoreCreateMutex();
  CDC::_interrupt_write_buffer_mutex = xSemaphoreCreateMutex();
  CDC::_tusb_state_eventgroup = xEventGroupCreate();

  // create tud_task() caller
  xTaskCreate(_usb_device_task, "usb::CDC::_tud_task_caller", 4096, NULL, 20,
              &_tud_task_handle);
  // create IRQ write flusher
  xTaskCreate(_IRQ_write_flusher, "usb::CDC::_IRQ_write_flusher", 4096, NULL,
              16, &_interrupt_write_task_handle);

  // tinyusb initialised, proceed
  return true;
}

bool CDC::wait_for_host_connection(u32 timeout) {
  if (xEventGroupWaitBits(_tusb_state_eventgroup, HOST_CONNECTED_BIT, pdFALSE,
                          pdTRUE, pdMS_TO_TICKS(timeout))) {
    return true;
  } else
    return false;
}

bool CDC::wait_for_CDC_connection(u32 timeout) {
  if (xEventGroupWaitBits(_tusb_state_eventgroup, CDC_CONNECTED_BIT, pdFALSE,
                          pdTRUE, pdMS_TO_TICKS(timeout))) {
    return true;
  } else
    return false;
}

bool CDC::host_connected(void) {
  return xEventGroupGetBits(_tusb_state_eventgroup) & HOST_CONNECTED_BIT;
}

bool CDC::CDC_connected(void) {
  return xEventGroupGetBits(_tusb_state_eventgroup) & CDC_CONNECTED_BIT;
}

bool CDC::write(const comms::SendIdentifiers identifier, const u8 *data,
                const u16 data_len) {
  // dont do anything if not connected
  if (!CDC_connected()) {
    return false;
  }
  // thread/task safety
  xSemaphoreTake(_write_mutex, portMAX_DELAY);

  u16 reported_len = data_len + sizeof(identifier);
  u16 packet_len = sizeof(reported_len) + reported_len;
  if (packet_len > MAX_TX_BUF_SIZE) {
    return false;
  }
  if (tud_cdc_available() < packet_len) {
    tud_cdc_write_flush(); // NOTE: this blocks (in tinyusb + rp2040), which we want.
  }
  // write to tusb CDC buffer
  tud_cdc_write(&reported_len, sizeof(reported_len));
  tud_cdc_write(&identifier, sizeof(identifier));
  tud_cdc_write(data, data_len);

  xSemaphoreGive(_write_mutex);

  return true;
}

bool CDC::write_from_IRQ(const comms::SendIdentifiers identifier,
                         const u8 *data, const u16 data_len,
                         BaseType_t *xHigherPriorityTaskWoken) {
  u16 reported_len = data_len + sizeof(identifier);
  u16 packet_len = sizeof(reported_len) + reported_len;
  // check for remaining space
  if (packet_len > MAX_INTERRUPT_TX_BUF_SIZE - _interrupt_write_buffer_index) {
    return false;
  }

  // interrupt/task/thread safety
  if (xSemaphoreTakeFromISR(_interrupt_write_buffer_mutex,
                            xHigherPriorityTaskWoken) != pdTRUE) {
    return false;
  }
  // write to interrupt write buffer
  _interrupt_write_buffer_write(&reported_len, sizeof(reported_len));
  _interrupt_write_buffer_write(&identifier, sizeof(identifier));
  _interrupt_write_buffer_write(data, data_len);

  xSemaphoreGiveFromISR(_interrupt_write_buffer_mutex,
                        xHigherPriorityTaskWoken);
  return true;
}

void CDC::flush_from_IRQ(BaseType_t *higher_priority_task_woken) {
  xTaskNotifyFromISR(_interrupt_write_task_handle, 0, eNoAction,
                     higher_priority_task_woken);
}

bool CDC::attach_listener(comms::RecvIdentifiers identifier,
                          TaskHandle_t handle, SemaphoreHandle_t mutex,
                          u8 *buffer, u16 length) {
  u8 idx = (u8)identifier;
  // check for existing listener
  if (_command_task_handles[idx]) {
    comms::CommsWarnings warn = comms::CommsWarnings::OVERRIDE_COMMAND_LISTENER;
    write(comms::SendIdentifiers::COMMS_WARN, (u8 *)&warn, sizeof(warn));
  }

  // check for param validity
  if (!(handle && mutex && buffer && length)) {
    comms::CommsErrors err = comms::CommsErrors::ATTACH_LISTENER_NULLPTR;
    write(comms::SendIdentifiers::COMMS_ERROR, (u8 *)&err, sizeof(err));
    return false;
  }

  // add listener
  _command_task_handles[idx] = handle;
  _command_task_buffer_mutexes[idx] = mutex;
  _command_task_buffers[idx] = buffer;
  _command_task_buffer_lengths[idx] = length;
  return true;
}

void CDC::_debug_printf(const char *format, ...) {
  char formatted[MAX_TX_BUF_SIZE];
  va_list args;
  va_start(args, format);
  u16 size = vsnprintf(formatted, sizeof(formatted), format, args);
  tud_cdc_write(formatted, size);
  tud_cdc_write_flush();
  va_end(args);
  return;
}

void CDC::_usb_device_task(void *args) {
  // initialise first, then loop
  tusb_init();
  xEventGroupSetBits(_tusb_state_eventgroup,
                     TUSB_INITED_BIT); // let other tasks start
  for (;;) {
    tud_task();
  }
}

void CDC::_IRQ_write_flusher(void *args) {
  for (;;) {
    // wait/make sure CDC is connected
    xEventGroupWaitBits(_tusb_state_eventgroup, CDC_CONNECTED_BIT, pdFALSE,
                        pdTRUE, portMAX_DELAY);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // block until notified
    // grab buffer access mutex (this will cause all write_from_IRQ executed while this is running to fail, but it is non-critical, so fine to fail.)
    // no critical section used as it is uh... non critical
    xSemaphoreTake(_interrupt_write_buffer_mutex, portMAX_DELAY);
    u16 remaining_len = _interrupt_write_buffer_index + 1;
    while (remaining_len > 0) {
      u16 writable = MIN(tud_cdc_available(), remaining_len);
      tud_cdc_write(_interrupt_write_buffer, writable);
      tud_cdc_write_flush();
      remaining_len -= writable;
    }
  }
}

// WARN: This does not execute in an interrupt context!
void CDC::_rx_cb(u8 interface, void *args) {
  // const u8 LED_PIN = 25;
  // gpio_put(LED_PIN, 1);
  // sleep_ms(500);
  // gpio_put(LED_PIN, 0);
  CurrentRXState &state = _current_rx_state;
  // prevention from missing out a command
  while (true) {
    // length bytes
    if (!state.length_bytes_recieved) {
      // wait until there are enough length bytes
      if (tud_cdc_available() < N_LENGTH_BYTES) {
        return;
      }
      // WARN: this assumes both systems have the same endianness. In the case of RPi and RP2040, both are little endian, so this is fine.
      tud_cdc_read(&state.expected_length, N_LENGTH_BYTES);
      state.length_bytes_recieved = true;
      if (state.expected_length > MAX_RX_BUF_SIZE) {
        // comms::CommsErrors err =
        //     comms::CommsErrors::PACKET_RECV_OVER_MAX_BUFSIZE;
        // write(comms::SendIdentifiers::COMMS_ERROR, (u8 *)&err, sizeof(err));
        _debug_printf("length over max buf size. recieved length: %u\n",
                      state.expected_length);
        state.reset();
        // WARN: after this, behavior becomes undefined
        // WARN: as we need to exit the cb for the error message to send, we cannot restart here.
        return;
      }
    }

    // data bytes
    else {
      _debug_printf("length recieved. expected length: %u\n",
                    state.expected_length);
      // wait until there are enough data bytes
      if (tud_cdc_available() < state.expected_length) {
        return;
      }
      // WARN: this assumes both systems have the same endianness. In the case of RPi and RP2040, both are little endian, so this is fine.
      tud_cdc_read(state.data_buffer, state.expected_length);
      // NOTE: here we have a full command in CurrentRXState.
      // this needs to be quickly copied into a command buffer.
      u8 identifier = state.data_buffer[0];
      _debug_printf("recieved identifier: %u\n", identifier);

      // check handler
      if (!_command_task_handles[identifier]) {
        // comms::CommsErrors err =
        //     comms::CommsErrors::CALLING_UNATTACHED_LISTENER;
        // u8 msg[] = {(u8)err, identifier};
        // write(comms::SendIdentifiers::COMMS_ERROR, msg, sizeof(msg));
        _debug_printf("unattached listener\n");
        state.reset();
        continue;
      }

      // check buffer mutex
      if (!_command_task_buffer_mutexes[identifier]) {
        // comms::CommsErrors err = comms::CommsErrors::LISTENER_NO_BUFFER_MUTEX;
        // u8 msg[] = {(u8)err, identifier};
        // write(comms::SendIdentifiers::COMMS_ERROR, msg, sizeof(msg));
        _debug_printf("no buffer mutex\n");
        state.reset();
        continue;
      }

      // check buffer
      if (!_command_task_buffers[identifier]) {
        // comms::CommsErrors err = comms::CommsErrors::LISTENER_NO_BUFFER;
        // u8 msg[] = {(u8)err, identifier};
        // write(comms::SendIdentifiers::COMMS_ERROR, msg, sizeof(msg));
        _debug_printf("no buffer\n");
        state.reset();
        continue;
      }

      // check length
      if (_command_task_buffer_lengths[identifier] <
          state.expected_length - 1) {
        // comms::CommsErrors err =
        //     comms::CommsErrors::PACKET_RECV_OVER_COMMAND_LISTENER_MAXSIZE;
        // u8 msg[] = {(u8)err, identifier};
        // write(comms::SendIdentifiers::COMMS_ERROR, msg, sizeof(msg));
        _debug_printf("packet size over command listener maxsize\n");
        state.reset();
        continue;
      }

      _debug_printf("trying to grab attached mutex\n");
      // try to grab buffer mutex
      if (xSemaphoreTake(_command_task_buffer_mutexes[identifier], 0) !=
          pdTRUE) {
        // mutex is already taken, drop this command
        // comms::CommsWarnings warn =
        //     comms::CommsWarnings::LISTENER_BUFFER_MUTEX_HELD;
        // u8 msg[] = {(u8)warn, identifier};
        // write(comms::SendIdentifiers::COMMS_WARN, msg, sizeof(msg));
        _debug_printf("cannot take mutex");
        state.reset();
        continue;
      }
      _debug_printf("grabbed mutex\n");

      // here we have the buffer mutex
      // clear the buffer first
      memset(_command_task_buffers[identifier], 0,
             _command_task_buffer_lengths[identifier]);
      // copy command into the buffer
      // state.data_buffer contains the identifier, so skip that when copying
      memcpy(_command_task_buffers[identifier],
             &state.data_buffer[sizeof(identifier)], // skip the identifier
             sizeof(state.data_buffer) - sizeof(identifier));
      // give the semaphore before notifying task, to avoid blocking
      xSemaphoreGive(_command_task_buffer_mutexes[identifier]);
      // notify task
      xTaskNotify(_command_task_handles[identifier], 0, eNoAction);

      state.reset();
      // NOTE: don't return here, to avoid missing out a command
      // NOTE: it should only return when there is not enough bytes in tud rx buffer
    }
  }
}

void CDC::_line_coding_cb(u8 interface, cdc_line_coding_t const *coding,
                          void *args) {
  // reset to bootloader with baud rate hack
  if (coding->bit_rate == 1200) {
    reset_usb_boot(0, 0);
  }
}

void CDC::_line_state_cb(u8 interface, bool dtr, bool rts, void *args) {
  if (dtr) {
    // connected
    xEventGroupSetBits(_tusb_state_eventgroup, CDC_CONNECTED_BIT);
    return;
  } else {
    // disconnected
    xEventGroupClearBits(_tusb_state_eventgroup, CDC_CONNECTED_BIT);
    return;
  }
}

bool CDC::_vendor_control_xfer_cb(u8 rhport, u8 stage,
                                  tusb_control_request_t const *request,
                                  void *args) {
  if (stage != CONTROL_STAGE_SETUP)
    return true;

  switch (request->bRequest) {
  case 0x01: // reset to bootloader
    reset_usb_boot(0, 0);
    return true;

  case 0x02: // restart
    watchdog_reboot(0, 0, 100);
    return true;
  }

  return false;
}

void CDC::_mount_cb(void *args) {
  xEventGroupSetBits(_tusb_state_eventgroup, HOST_CONNECTED_BIT);
  return;
}

void CDC::_unmount_cb(void *args) {
  xEventGroupClearBits(_tusb_state_eventgroup, HOST_CONNECTED_BIT);
}

} // namespace usb
