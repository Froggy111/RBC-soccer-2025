#pragma once
#include "types.hpp"
#include "dbg_pins.hpp"
#include "pin_selector.hpp"

class MotorDriver {
private:
  void init_spi(types::u16 SPI_SPEED);
  void init_pins();

  // * register reading
  uint8_t read8(types::u8 reg);
  void write8(types::u8 reg, types::u8 data, types::u8 mask = 0xFF);

  // * specific registers
  void config_registers(); // setup the registers
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
  void init(types::u8 id, types::u16 SPI_SPEED);

  // activate the driver (nsleep to 1)
  void set_activate(bool activate);

  // command a speed and direction
  bool command(types::u16 speed, bool direction);

  // handle error
  void handle_error(void* _);
};