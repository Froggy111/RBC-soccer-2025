#pragma once
#include "libs/utils/types.hpp"
#include "pico/stdio.h"
#include "pinmap.hpp"
#include "pins.hpp"

class MotorDriver {
private:
  void init_spi(types::u16 SPI_SPEED);
  void init_pins();
  void init_registers_through_spi();

  static uint8_t read8(types::u8 reg);
  static void write8(types::u8 reg, types::u8 data);

  void handle_error(uint gpio, uint32_t events);

  PinInputControl inputControl;
  PinOutputControl outputControl;

public:
  void init(types::u8 id, types::u16 SPI_SPEED);
  void command(types::u16 speed, bool direction);
};