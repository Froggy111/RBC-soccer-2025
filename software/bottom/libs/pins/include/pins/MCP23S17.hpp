#pragma once

#include "pinmap.hpp"
#include "types.hpp"
#include "pins/types.hpp"

extern "C" {
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
}

namespace pins {

class MCP23S17 {
public:
  MCP23S17(types::u8 SCLK, types::u8 MISO, types::u8 MOSI, types::u8 SCS,
           types::u8 RESET, types::u8 address, bool int_from_isr,
           spi_inst_t *spi_obj);
  // WARN: MUST BE CALLED AFTER SCHEDULER START
  void init();

  void reset();

  void set_pin_mode(types::u8 pin, bool on_A, DigitalPinMode pinmode);
  void write(types::u8 pin, bool on_A, bool value);
  bool read(types::u8 pin, bool on_A);

  bool attach_interrupt(types::u8 pin, bool on_A,
                        DigitalPinInterrupt interrupt_handler,
                        DigitalPinInterruptState interrupt_state, void *args);
  // attach this to external, just notifies interrupt_handler_task
  void interrupt_handler(void *args);

  // these should be private, but are public for static funcs to use
  types::u8 read8(types::u8 reg_address);
  void write8(types::u8 reg_address, types::u8 data, types::u8 mask = 0xFF);
  DigitalPinInterrupt _interrupt_handlers[16] = {nullptr};
  DigitalPinInterruptState _interrupt_states[16] = {
      DigitalPinInterruptState::EDGE_FALL};
  void *_interrupt_handler_args[16] = {nullptr};

private:
  types::u8 _pin_state[16];
  spi_inst_t *_spi_obj = nullptr;
  bool _initialised = false;
  types::u8 _SCLK = 255;
  types::u8 _MISO = 255;
  types::u8 _MOSI = 255;
  types::u8 _SCS = 255;
  types::u8 _INT = 255;
  types::u8 _RESET = 255;
  types::u8 _address = 255;
  bool _int_from_isr = false;
  TaskHandle_t interrupt_handler_task_handle = nullptr;

  /**
   * @brief Init the fSPI interface. Calls configure_spi and sets any registers it needs to.
   * 
   */
  void init_spi();

  void init_pins();

  void configure_spi();

  // params is this
  static void interrupt_handler_task(void *params);
};

} // namespace pins
