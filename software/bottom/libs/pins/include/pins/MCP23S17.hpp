#pragma once

#include "pinmap.hpp"
#include "types.hpp"
#include "pins/types.hpp"

extern "C" {
#include <hardware/spi.h>
#include <pico/stdlib.h>
}

namespace pins {

class MCP23S17 {
public:
  void init(types::u8 SCLK, types::u8 MISO, types::u8 MOSI, types::u8 SCS,
            types::u8 RESET, types::u8 address, bool int_from_isr,
            spi_inst_t *spi_obj);

  void reset();

  void set_pin_mode(types::u8 pin, bool on_A, DigitalPinMode pinmode);
  void write(types::u8 pin, bool on_A, bool value);
  bool read(types::u8 pin, bool on_A);

  void attach_interrupt(types::u8 pin, bool on_A, DigitalPinInterrupt,
                        DigitalPinInterruptState);
  // attach this to external, just notifies interrupt_handler_task
  void interrupt_handler(void *params);
  void interrupt_handler_task(void *params);

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

  DigitalPinInterrupt _interrupt_funcs[16] = {nullptr};

  /**
   * @brief Init the fSPI interface. Calls configure_spi and sets any registers it needs to.
   * 
   */
  void init_spi();

  void init_pins();

  void configure_spi();

  types::u8 read8(types::u8 device_address, types::u8 reg_address);

  void write8(types::u8 device_address, types::u8 reg_address, types::u8 data,
              types::u8 mask = 0xFF);
};

} // namespace pins
