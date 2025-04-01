#include "comms.hpp"
#include "comms/uart.hpp"
#include "comms/usb.hpp"
#include "types.hpp"

/**
 * INFO:
 * IMPORTANT: RP2040 and RPi are all little endian (least significant byte first)
 * Communication format is defined as such, in both directions:
 * Byte 1 & 2: Length (least significant byte first) (excludes length bytes)
 * Byte 3: board_id (u8 enum)
 * The rest is passed to the specific handler.
 */

using namespace types;

namespace comms {

usb::CDC USB_CDC = usb::CDC();
uart::Serial UART_serial = uart::Serial();

bool init(void) {
  USB_CDC.init();
  ping_task_mutex = xSemaphoreCreateMutex();
  xTaskCreate(ping_task, "ping_task", PING_TASK_STACK_DEPTH, nullptr,
              PING_TASK_PRIORITY, &ping_task_handle);
  USB_CDC.attach_listener(RecvIdentifiers::PING, ping_task_handle,
                          ping_task_mutex, ping_task_buffer,
                          sizeof(ping_task_buffer));

  board_id_task_mutex = xSemaphoreCreateMutex();
  xTaskCreate(board_id_task, "board_id_task", board_id_TASK_STACK_DEPTH,
              nullptr, board_id_TASK_PRIORITY, &board_id_task_handle);
  USB_CDC.attach_listener(RecvIdentifiers::BOARD_ID, board_id_task_handle,
                          board_id_task_mutex, board_id_task_buffer,
                          sizeof(board_id_task_buffer));

  return true;
}

void ping_task(void *params) {
  // ignore params
  for (;;) {
    // take in data
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(ping_task_mutex, portMAX_DELAY);
    memcpy(ping_task_data, ping_task_buffer, sizeof(ping_task_data));
    memset(ping_task_buffer, 0, sizeof(ping_task_buffer));
    xSemaphoreGive(ping_task_mutex);

    USB_CDC.write(SendIdentifiers::PING, ping_task_data,
                  sizeof(ping_task_data));
    memset(ping_task_data, 0, sizeof(ping_task_data));
  }
}

void board_id_task(void *params) {
  // ignore params

  for (;;) {
    // data-less command
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xSemaphoreTake(board_id_task_mutex, portMAX_DELAY);
    xSemaphoreGive(board_id_task_mutex);

#ifdef IS_BOTTOM_PICO
    BoardIdentifiers board_id = BoardIdentifiers::BOTTOM_PICO;
#elif defined(IS_MIDDLE_PICO)
    BoardIdentifiers board_id = BoardIdentifiers::MIDDLE_PICO;
#elif defined(IS_TOP_PICO)
    BoardIdentifiers board_id = BoardIdentifiers::TOP_PICO;
#endif

    USB_CDC.write(SendIdentifiers::BOARD_ID, (u8 *)&board_id, sizeof(board_id));
  }
}

} // namespace comms
