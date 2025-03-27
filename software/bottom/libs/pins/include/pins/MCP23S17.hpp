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
  void init(types::u8 device_id, spi_inst_t *spi_obj);

  void reset();

  void init_gpio(types::u8 pin, bool on_A, DigitalPinMode pinmode);
  void write_gpio(types::u8 pin, bool on_A, bool value);
  bool read_gpio(types::u8 pin, bool on_A);

  void attach_interrupt(types::u8 pin, bool on_A, DigitalPinInterrupt,
                        DigitalPinInterruptState);

private:
  types::u8 id;
  types::u8 pin_state[17];
  spi_inst_t *spi_obj;

  static bool initialized[2];

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
