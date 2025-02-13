#pragma once
#include "libs/utils/types.hpp"
#include "pico/stdio.h"
#include "dbg_pins.hpp"

class MotorDriver {
private:
  void init_spi(types::u16 SPI_SPEED);
  void init_pins();
  void init_registers_through_spi();

  uint8_t read8(types::u8 reg);
  void write8(types::u8 reg, types::u8 data);

  PinInputControl inputControl;
  PinOutputControl outputControl;
  std::map<std::string, types::u8> pinmap;

public:
  void init(types::u8 id, types::u16 SPI_SPEED);
  void command(types::u16 speed, bool direction);
  void handle_error(void* _);
};