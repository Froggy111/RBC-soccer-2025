#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include "IR.hpp"

extern "C" {
#include <hardware/timer.h>
#include <hardware/irq.h>
}

using namespace types;

namespace IR {

void init(void) {
  debug::info("Initializing IR sensors");
  for (u8 i = 0; i < SENSOR_COUNT; i++) {
    pulse_data[i].reset();
    modulation_data[i].reset();
    u8 pin = SENSOR_PINS[i];
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
  }
  xTaskCreate(modulation_handler_task, "modulation handler",
              MODULATION_HANLDER_TASK_STACK_SIZE, nullptr,
              MODULATION_HANLDER_TASK_PRIORITY,
              &modulation_handler_task_handle);
  gpio_set_irq_callback(pulse_handler);

  // setup modulation timer
  hw_set_bits(&timer_hw->inte, 0b1 << MODULATION_ALARM_IDX);
  irq_set_exclusive_handler(MODULATION_IRQ, modulation_handler);
  irq_set_enabled(MODULATION_IRQ, true);
}

void pulse_handler_individual(u8 id, u32 event) {
  if (event & GPIO_IRQ_EDGE_FALL) { // IR activation
    pulse_data[id].last_fall = time_us_64();
  } else if (event & GPIO_IRQ_EDGE_RISE) {
    pulse_data[id].uptime += time_us_64() - pulse_data[id].last_fall;
  }
  return;
}

void modulation_handler(void) {
  BaseType_t higher_priority_task_woken;
  // setup time
  u32 current_alarm_target = timer_hw->alarm[MODULATION_ALARM_IDX];
  hw_clear_bits(&timer_hw->intr, 0b1 << MODULATION_ALARM_IDX);

  u32 next_alarm_target = current_alarm_target + CYCLES_PER_MODULATION;
  timer_hw->alarm[MODULATION_ALARM_IDX] = next_alarm_target;

  for (u8 i = 0; i < SENSOR_COUNT; i++) {
    // copy all the current uptimes over and zero pulse data
    modulation_data[i].uptime = pulse_data[i].uptime;
    pulse_data[i].zero();
  }
  xTaskNotifyFromISR(modulation_handler_task_handle, 0, eNoAction,
                     &higher_priority_task_woken);
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
  switch (pin) {
  case pinmap::Pico::IR1:
    pulse_handler_individual(0, events);
    break;
  case pinmap::Pico::IR2:
    pulse_handler_individual(1, events);
    break;
  case pinmap::Pico::IR3:
    pulse_handler_individual(2, events);
    break;
  case pinmap::Pico::IR4:
    pulse_handler_individual(3, events);
    break;
  case pinmap::Pico::IR5:
    pulse_handler_individual(4, events);
    break;
  case pinmap::Pico::IR6:
    pulse_handler_individual(5, events);
    break;
  case pinmap::Pico::IR7:
    pulse_handler_individual(6, events);
    break;
  case pinmap::Pico::IR8:
    pulse_handler_individual(7, events);
    break;
  case pinmap::Pico::IR9:
    pulse_handler_individual(8, events);
    break;
  case pinmap::Pico::IR10:
    pulse_handler_individual(9, events);
    break;
  case pinmap::Pico::IR11:
    pulse_handler_individual(10, events);
    break;
  case pinmap::Pico::IR12:
    pulse_handler_individual(11, events);
    break;
  case pinmap::Pico::IR13:
    pulse_handler_individual(12, events);
    break;
  case pinmap::Pico::IR14:
    pulse_handler_individual(13, events);
    break;
  case pinmap::Pico::IR15:
    pulse_handler_individual(14, events);
    break;
  case pinmap::Pico::IR16:
    pulse_handler_individual(15, events);
    break;
  case pinmap::Pico::IR17:
    pulse_handler_individual(16, events);
    break;
  case pinmap::Pico::IR18:
    pulse_handler_individual(17, events);
    break;
  case pinmap::Pico::IR19:
    pulse_handler_individual(18, events);
    break;
  case pinmap::Pico::IR20:
    pulse_handler_individual(19, events);
    break;
  case pinmap::Pico::IR21:
    pulse_handler_individual(20, events);
    break;
  case pinmap::Pico::IR22:
    pulse_handler_individual(21, events);
    break;
  case pinmap::Pico::IR23:
    pulse_handler_individual(22, events);
    break;
  case pinmap::Pico::IR24:
    pulse_handler_individual(23, events);
    break;
  default:
    // something went wrong
    break;
  }
  return;
}

} // namespace IR
