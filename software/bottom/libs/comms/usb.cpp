#include "comms/usb.hpp"
#include "comms/errors.hpp"
#include "comms/identifiers.hpp"
#include "types.hpp"
extern "C" {
#include <FreeRTOS.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <tusb.h>
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
}

using namespace types;

namespace usb {

CurrentRXState state;

CDC::CDC() {
  // mostly stuff that should be set up before initialising tinyusb
  // set tusb callback handlers
  usb::vendor_control_xfer_cb_fn = _vendor_control_xfer_cb;
  usb::vendor_control_xfer_cb_user_args = nullptr;
  usb::CDC_line_coding_cb_fn = _line_coding_cb;
  usb::CDC_line_coding_cb_user_args = nullptr;
  usb::CDC_rx_cb_fn = _rx_cb;
  usb::CDC_rx_cb_user_args = &_current_rx_state;

  // setup buffers
  _current_rx_state.data_buffer = _read_buffer;
  _current_rx_state.reset();

  // create some tasks
}

bool CDC::init(void) {
  if (tusb_init()) {
    return false; // some error, idk
  }
  while (!tusb_inited()) {
    tud_task();
    sleep_us(TUSB_INIT_SLEEP_LOOP_TIME);
  }

  // tinyusb initialised, proceed
  _initialised = true;

  // create tud_task() caller
  xTaskCreate(_tud_task_caller, "usb::CDC::_tud_task_caller", 4096, NULL, 20,
              NULL);
}

bool CDC::wait_for_host_connection(u32 timeout) {
  u64 t_start = time_us_64();
  do {
  } while (tud_connected() && time_us_64() - t_start < timeout);
  if (tud_connected()) {
    return true;
  }
  return false;
}

bool CDC::wait_for_CDC_connection(u32 timeout) {
  u64 t_start = time_us_64();
  do {
  } while (tud_cdc_connected() && time_us_64() - t_start < timeout);
  if (tud_cdc_connected()) {
    return true;
  }
  return false;
}

bool CDC::write(const comms::SendIdentifiers identifier, const u8 *data,
                const u16 data_len) {
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

  return true;
}

u32 CDC::read(u8 *data, u32 len, u32 timeout) {
  u64 t_start = time_us_64();
  u32 n_read = 0;

  while (time_us_64() - t_start < timeout) {
    int read = stdio_getchar();
    if (read < 0) { // EOF or some other weird thing
      continue;
    }
    data[n_read] = (u8)read;
    n_read++;
  }
}

void CDC::_line_coding_cb(u8 interface, cdc_line_coding_t const *coding,
                          void *args) {
  // Handle reset via baud rate if needed
  if (coding->bit_rate == 1200) {
    reset_usb_boot(0, 0);
  }
}

bool CDC::_vendor_control_xfer_cb(u8 rhport, u8 stage,
                                  tusb_control_request_t const *request,
                                  void *args) {
  if (stage != CONTROL_STAGE_SETUP)
    return true;

  switch (request->bRequest) {
  case 0x01: // Reset to bootloader
    reset_usb_boot(0, 0);
    return true;

  case 0x02: // Reset to application
    watchdog_reboot(0, 0, 100);
    return true;
  }

  return false;
}
void CDC::_rx_cb(u8 interface, void *args) {
  CDC *_this = (CDC *)args;
  CurrentRXState &state = _this->_current_rx_state;
  CurrentRawCommand &command = _this->_current_command;

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
      if (state.expected_length > MAX_RX_BUF_SIZE) {
        comms::CommsErrors err = comms::CommsErrors::PACKET_RECV_TOO_LONG;
        interrupt_write(comms::SendIdentifiers::COMMS_ERROR, (u8 *)&err,
                        sizeof(err));
        interrupt_flush();
        state.reset();
        // WARN: after this, behavior becomes undefined
        // WARN: as we need to exit the cb for the error message to send, we cannot restart here.
        return;
      }
    }

    // data bytes
    if (!state.recieved) {
      // wait until there are enough data bytes
      if (tud_cdc_available() < state.expected_length) {
        return;
      }
      // WARN: this assumes both systems have the same endianness. In the case of RPi and RP2040, both are little endian, so this is fine.
      tud_cdc_read(state.data_buffer, state.expected_length);
      // NOTE: here we have a full command in CurrentRXState.
      // this needs to be quickly copied into a command buffer.
      state.reset();
      return;
    }
  }
}

void CDC::_rx_command_process_task(void *args) {
  BaseType_t task_notification_result;
  for (;;) {
    task_notification_result = xTaskNotifyWait(pdFALSE, ULONG_MAX,
  }
}

} // namespace usb
