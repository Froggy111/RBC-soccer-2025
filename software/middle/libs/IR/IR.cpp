#include "comms.hpp"
#include "debug.hpp"
#include "portmacro.h"
#include "projdefs.h"
#include "types.hpp"
#include <hardware/regs/intctrl.h>
#include "IR.hpp"

extern "C" {
#include <hardware/timer.h>
#include <hardware/irq.h>
}
using namespace types;

namespace IR {

void init(void) {
  debug::info("Initializing IR sensors");
  irq_set_priority(MODULATION_IRQ, 0x40);
  irq_set_priority(IO_IRQ_BANK0, 0x80);
  for (u8 i = 0; i < SENSOR_COUNT; i++) {
    pulse_data[i].reset();
    modulation_data[i].reset();
    u8 pin = SENSOR_PINS[i];
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    gpio_set_irq_enabled_with_callback(
        pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, pulse_handler);
  }
  xTaskCreate(modulation_handler_task, "modulation handler",
              MODULATION_HANLDER_TASK_STACK_SIZE, nullptr,
              MODULATION_HANLDER_TASK_PRIORITY,
              &modulation_handler_task_handle);
  irq_set_enabled(IO_IRQ_BANK0, true);

  // setup modulation timer
  hw_set_bits(&timer_hw->inte, 0b1 << MODULATION_ALARM_IDX);
  irq_set_exclusive_handler(MODULATION_IRQ, modulation_handler);
  irq_set_enabled(MODULATION_IRQ, true);

  // Set initial alarm value to trigger first interrupt
  timer_hw->alarm[MODULATION_ALARM_IDX] =
      timer_hw->timerawl + US_PER_MODULATION;
}

void pulse_handler_individual(u8 id, u32 event) {
  if (event & GPIO_IRQ_EDGE_FALL) { // IR activation
    pulse_data[id].last_fall = time_us_64();
  } else if (event & GPIO_IRQ_EDGE_RISE) {
    u32 time_passed = time_us_64() - pulse_data[id].last_fall;
    if (time_passed < max_time_passed) {
      pulse_data[id].uptime += time_passed;
    }
  }
  return;
}

void modulation_handler(void) {
  BaseType_t higher_priority_task_woken = pdFALSE;
  // setup time
  u32 current_alarm_target = timer_hw->alarm[MODULATION_ALARM_IDX];
  hw_clear_bits(&timer_hw->intr, 0b1 << MODULATION_ALARM_IDX);

  u32 next_alarm_target = current_alarm_target + US_PER_MODULATION;
  timer_hw->alarm[MODULATION_ALARM_IDX] = next_alarm_target;

  for (u8 i = 0; i < SENSOR_COUNT; i++) {
    // copy all the current uptimes over and zero pulse data
    modulation_data[i].uptime = pulse_data[i].uptime;
    pulse_data[i].reset();
  }
  xTaskNotifyFromISR(modulation_handler_task_handle, 0, eNoAction,
                     &higher_priority_task_woken);
  gpio_put(comms::LED_PIN, !gpio_get(comms::LED_PIN));
  portYIELD_FROM_ISR(higher_priority_task_woken);
}

void modulation_handler_task(void *params) {
  u8 buffer[sizeof(modulation_data)] = {0};
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    memcpy((void *)buffer, (void *)modulation_data, sizeof(buffer));
    comms::USB_CDC.write(comms::SendIdentifiers::IR_DATA, buffer,
                         sizeof(buffer));
  }
}

void pulse_handler(uint gpio, u32 events) {
  pinmap::Pico pin = (pinmap::Pico)gpio;
  // BaseType_t higher_priority_task_woken = pdFALSE;
  switch (pin) {
  case pinmap::Pico::IR1: {
    // u8 msg[] = "IR1 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg,
    //                               sizeof(msg), &higher_priority_task_woken);
    pulse_handler_individual(0, events);
    break;
  }
  case pinmap::Pico::IR2: {
    // u8 msg2[] = "IR2 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg2,
    //                               sizeof(msg2), &higher_priority_task_woken);
    pulse_handler_individual(1, events);
    break;
  }
  case pinmap::Pico::IR3: {
    // u8 msg3[] = "IR3 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg3,
    //                               sizeof(msg3), &higher_priority_task_woken);
    pulse_handler_individual(2, events);
    break;
  }
  case pinmap::Pico::IR4: {
    // u8 msg4[] = "IR4 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg4,
    //                               sizeof(msg4), &higher_priority_task_woken);
    pulse_handler_individual(3, events);
    break;
  }
  case pinmap::Pico::IR5: {
    // u8 msg5[] = "IR5 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg5,
    //                               sizeof(msg5), &higher_priority_task_woken);
    pulse_handler_individual(4, events);
    break;
  }
  case pinmap::Pico::IR6: {
    // u8 msg6[] = "IR6 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg6,
    //                               sizeof(msg6), &higher_priority_task_woken);
    pulse_handler_individual(5, events);
    break;
  }
  case pinmap::Pico::IR7: {
    // u8 msg7[] = "IR7 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg7,
    //                               sizeof(msg7), &higher_priority_task_woken);
    pulse_handler_individual(6, events);
    break;
  }
  case pinmap::Pico::IR8: {
    // u8 msg8[] = "IR8 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg8,
    //                               sizeof(msg8), &higher_priority_task_woken);
    pulse_handler_individual(7, events);
    break;
  }
  case pinmap::Pico::IR9: {
    // u8 msg9[] = "IR9 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg9,
    //                               sizeof(msg9), &higher_priority_task_woken);
    pulse_handler_individual(8, events);
    break;
  }
  case pinmap::Pico::IR10: {
    // u8 msg10[] = "IR10 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg10,
    //                               sizeof(msg10), &higher_priority_task_woken);
    pulse_handler_individual(9, events);
    break;
  }
  case pinmap::Pico::IR11: {
    // u8 msg11[] = "IR11 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg11,
    //                               sizeof(msg11), &higher_priority_task_woken);
    pulse_handler_individual(10, events);
    break;
  }
  case pinmap::Pico::IR12: {
    // u8 msg12[] = "IR12 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg12,
    //                               sizeof(msg12), &higher_priority_task_woken);
    pulse_handler_individual(11, events);
    break;
  }
  case pinmap::Pico::IR13: {
    // u8 msg13[] = "IR13 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg13,
    //                               sizeof(msg13), &higher_priority_task_woken);
    pulse_handler_individual(12, events);
    break;
  }
  case pinmap::Pico::IR14: {
    // u8 msg14[] = "IR14 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg14,
    //                               sizeof(msg14), &higher_priority_task_woken);
    pulse_handler_individual(13, events);
    break;
  }
  case pinmap::Pico::IR15: {
    // u8 msg15[] = "IR15 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg15,
    //                               sizeof(msg15), &higher_priority_task_woken);
    pulse_handler_individual(14, events);
    break;
  }
  case pinmap::Pico::IR16: {
    // u8 msg16[] = "IR16 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg16,
    //                               sizeof(msg16), &higher_priority_task_woken);
    pulse_handler_individual(15, events);
    break;
  }
  case pinmap::Pico::IR17: {
    // u8 msg17[] = "IR17 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg17,
    //                               sizeof(msg17), &higher_priority_task_woken);
    pulse_handler_individual(16, events);
    break;
  }
  case pinmap::Pico::IR18: {
    // u8 msg18[] = "IR18 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg18,
    //                               sizeof(msg18), &higher_priority_task_woken);
    pulse_handler_individual(17, events);
    break;
  }
  case pinmap::Pico::IR19: {
    // u8 msg19[] = "IR19 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg19,
    //                               sizeof(msg19), &higher_priority_task_woken);
    pulse_handler_individual(18, events);
    break;
  }
  case pinmap::Pico::IR20: {
    // u8 msg20[] = "IR20 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg20,
    //                               sizeof(msg20), &higher_priority_task_woken);
    pulse_handler_individual(19, events);
    break;
  }
  case pinmap::Pico::IR21: {
    // u8 msg21[] = "IR21 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg21,
    //                               sizeof(msg21), &higher_priority_task_woken);
    pulse_handler_individual(20, events);
    break;
  }
  case pinmap::Pico::IR22: {
    // u8 msg22[] = "IR22 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg22,
    //                               sizeof(msg22), &higher_priority_task_woken);
    pulse_handler_individual(21, events);
    break;
  }
  case pinmap::Pico::IR23: {
    // u8 msg23[] = "IR23 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg23,
    //                               sizeof(msg23), &higher_priority_task_woken);
    pulse_handler_individual(22, events);
    break;
  }
  case pinmap::Pico::IR24: {
    // u8 msg24[] = "IR24 triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG, msg24,
    //                               sizeof(msg24), &higher_priority_task_woken);
    pulse_handler_individual(23, events);
    break;
  }
  default: {
    // u8 msg_default[] = "IR not triggered\r\n";
    // comms::USB_CDC.write_from_IRQ(comms::SendIdentifiers::COMMS_DEBUG,
    //                               msg_default, sizeof(msg_default),
    // &higher_priority_task_woken);
    // something went wrong
    break;
  }
  }
  // comms::USB_CDC.flush_from_IRQ(&higher_priority_task_woken);
  // portYIELD_FROM_ISR(higher_priority_task_woken);
}

} // namespace IR
