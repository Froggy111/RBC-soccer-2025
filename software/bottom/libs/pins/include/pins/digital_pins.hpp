#pragma once

#include "pinmap.hpp"
#include "pins/MCP23S17.hpp"
#include "types.hpp"
#include "pins/types.hpp"

extern "C" {
#include "hardware/gpio.h"
#include <pico/stdlib.h>
}

namespace pins {

const types::u8 DMUX_1_ADDRESS = 0b000;
const types::u8 DMUX_2_ADDRESS = 0b001;

// must use this to attach an interrupt
#define make_handler(HANDLER_NAME)                                             \
  [this](void *runtime_arg) { this->HANDLER_NAME(runtime_arg); }

class DigitalPins {
public:
  DigitalPins() = default;
  /**
   * @brief initialise DMUX, GPIOs, etc.
   * @warning MUST be called before any other class functions (checks will be made)
   */
  bool init(void);
  bool set_mode(pinmap::Digital pin, DigitalPinMode pin_mode);
  bool read(pinmap::Digital pin);
  bool write(pinmap::Digital pin, bool val);
  /**
   * @brief attach interrupt to digital pin
   * @param pin: digital pin selection
   * @param interrupt_state: interrupt state to trigger interrupt on
   * @param interrupt_handler: function to be run for this interrupt condition. must call make_handler(function) on any non-static class functions
   * @param args: custom user args to be passed
   */
  // bool attach_interrupt(pinmap::Digital pin,
  //                       DigitalPinInterruptState interrupt_state,
  //                       const DigitalPinInterrupt interrupt_handler,
  //                       void *args);
  // bool detach_interrupt(pinmap::Digital pin);
  // bool enable_interrupt(pinmap::Digital pin);
  // bool disable_interrupt(pinmap::Digital pin);
  //
  // // should be private but called externally so...
  // // reason is pico gpio interrupt type doesnt allow user args
  // void pico_gpio_interrupt_handler(types::u32 gpio, types::u32 event);

private:
  MCP23S17 _dmux_1 = MCP23S17((types::u8)pinmap::Digital::SPI0_SCLK,
                              (types::u8)pinmap::Digital::SPI0_MISO,
                              (types::u8)pinmap::Digital::SPI0_MOSI,
                              (types::u8)pinmap::Digital::DMUX_SCS,
                              (types::u8)pinmap::Pico::DMUX_RESET,
                              DMUX_1_ADDRESS, false, spi0);
  MCP23S17 _dmux_2 =
      MCP23S17((types::u8)pinmap::Digital::SPI0_SCLK,
               (types::u8)pinmap::Digital::SPI0_MISO,
               (types::u8)pinmap::Digital::SPI0_MOSI,
               (types::u8)pinmap::Digital::DMUX_SCS,
               (types::u8)pinmap::Pico::DMUX_RESET, DMUX_2_ADDRESS, true, spi0);
  DigitalPinInterrupt _interrupts[pinmap::digital_pin_count] = {nullptr};
  DigitalPinInterruptState _interrupt_states[pinmap::digital_pin_count] = {
      DigitalPinInterruptState::EDGE_FALL};
  void *_interrupt_args[pinmap::digital_pin_count] = {nullptr};

  DigitalPinMode _pin_modes[pinmap::digital_pin_count] = {
      DigitalPinMode::INPUT};
  bool _initialized = false;
};

extern DigitalPins digital_pins;
// static inline void pico_gpio_interrupt_handler_wrapper(uint gpio,
//                                                        uint32_t event) {
//   digital_pins.pico_gpio_interrupt_handler(gpio, event);
// }

} // namespace pins
