#pragma once

#include "types.hpp"
#include "pinmap.hpp"

extern "C" {
#include <pico/stdlib.h>
}

namespace pins {

// NOTE: INPUT_PULLDOWN is not defined as MCP23S17 only has internal pullups, not pulldowns
// NOTE: If INPUT_PULLUP functionality is needed, inform me.
enum class DigitalPinMode : types::u8 { INPUT, OUTPUT, INPUT_PULLUP };
enum class DigitalPinInterruptState : types::u8 {
  EDGE_RISE = GPIO_IRQ_EDGE_RISE,
  EDGE_FALL = GPIO_IRQ_EDGE_FALL,
  LEVEL_HIGH = GPIO_IRQ_LEVEL_HIGH,
  LEVEL_LOW = GPIO_IRQ_LEVEL_LOW
};

using DigitalPinInterrupt = void (*)(DigitalPinInterruptState, void *);

} // namespace pins
