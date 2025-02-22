#pragma once
#include "types.hpp"
#include "dbg_pins.hpp"
#include "pin_selector.hpp"
#include <pico/types.h>
#include <string>

class MotorDriver {
private:
  void init_spi(types::u64 SPI_SPEED);
  void init_pins();

  // * register reading
  types::u8 read8(types::u8 reg);
  bool write8(types::u8 reg, types::u8 data, int8_t expected = -1); // TODO: Add Mask

  // * specific registers
  void set_registers(); // setup the registers
  bool check_registers(); // check if the registers are configured correctly
  std::string read_fault_summary();
  std::string read_status1();
  std::string read_status2();

  //* others
  bool check_config();

  PinInputControl inputControl;
  PinOutputControl outputControl;
  PinSelector pinSelector;

public:
  void init(int id, types::u64 SPI_SPEED);
  bool init_registers();

  // command a speed and direction
  bool command(types::i16 duty_cycle);

  // handle error
  static void handle_error(MotorDriver *driver);
};