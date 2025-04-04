#pragma once
#include "comms/uart.hpp"
#include "comms/usb.hpp"
#include "types.hpp"
#include "config.hpp"

/**
 * INFO:
 * IMPORTANT: RP2040 and RPi are all little endian (least significant byte first)
 * Communication format is defined as such, in both directions:
 * Byte 1 & 2: Length (least significant byte first) (excludes length bytes)
 * Byte 3: board_id (u8 enum)
 * The rest is passed to the specific handler.
 */

namespace comms {

extern usb::CDC USB_CDC;
extern uart::Serial UART_serial;

// initialises communication interfaces
bool init(void);

// ping task
static void ping_task(void *params);
static SemaphoreHandle_t ping_task_mutex;
static types::u8 ping_task_buffer[PING_PACKET_MAX_SIZE];
static types::u8 ping_task_data[PING_PACKET_MAX_SIZE];
static TaskHandle_t ping_task_handle;
const types::u16 PING_TASK_STACK_DEPTH = 256;
const types::u8 PING_TASK_PRIORITY = 16;

// board_id task
static void board_id_task(void *params);
static SemaphoreHandle_t board_id_task_mutex;
static TaskHandle_t board_id_task_handle;
static types::u8 board_id_task_buffer[1]; // just to not fail nullptr checks
const types::u16 BOARD_ID_TASK_STACK_DEPTH = 256;
const types::u8 BOARD_ID_TASK_PRIORITY = 16;

// blink task
const types::u8 LED_PIN = 25;

struct BlinkTaskData {
  types::u16 blink_time_ms = 0;
  void reset(void) { blink_time_ms = 0; }
};
static TaskHandle_t blink_task_handle;
static BlinkTaskData blink_task_data;
static types::u8 blink_task_buffer[sizeof(BlinkTaskData)];
static SemaphoreHandle_t blink_task_mutex;
const types::u16 BLINK_TASK_STACK_DEPTH = 256;
const types::u8 BLINK_TASK_PRIORITY = 16;

void blink_task(void *args);

} // namespace comms
