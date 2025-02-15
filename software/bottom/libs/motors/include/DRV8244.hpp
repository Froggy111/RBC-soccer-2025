#pragma once
#include "types.hpp"
#include "dbg_pins.hpp"
extern "C" {
#include <pico/stdlib.h>
}

class MotorDriver {
private:
  void init_spi(types::u16 SPI_SPEED);
  void init_pins();
  void init_registers_through_spi();

  // * register reading
  uint8_t read8(types::u8 reg);
  void write8(types::u8 reg, types::u8 data);

  // * specific registers
  std::string read_fault_summary();
  std::string read_status1();
  std::string read_status2();

  PinInputControl inputControl;
  PinOutputControl outputControl;
  PinSelector pinSelector;

public:
  void init(types::u8 id, types::u16 SPI_SPEED);
  void command(types::u16 speed, bool direction);
  void handle_error(void* _);
};