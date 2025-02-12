#pragma once
#include "hardware/gpio.h"
#include "hardware/spi.h
#include "libs/hardware-descriptors/pinmap.hpp"
#include "libs/utils/types.hpp"
#include <pico/stdlib.h>

namespace Pins {

enum PinMode { INPUT, OUTPUT };

class DigitalPin {
public:
  void change_mode(PinMode mode, types::u8 pin);
  bool read(types::u8 pin);
  void write(types::u8 pin, bool value);
};

class AnalogPin {
  
};
}