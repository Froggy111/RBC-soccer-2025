#pragma once
#include "types.hpp"
#include "dbg_pins.hpp"
#include "pin_selector.hpp"
#include <hardware/spi.h>
#include <pico/types.h>
#include <string>

class MouseSensor {
public:
  void init(int id, spi_inst_t *spi_obj_touse);

  types::u8 motion_burst_buffer[12] = {99}; //Stores data read from data burst

  void read_motion_burst();
  types::i16 read_X_motion();
  types::i16 read_Y_motion();
  types::u8 read_squal();

private:
  //* init funcs
  void init_pins();
  bool init_registers();
  bool init_srom();

  types::u8 read8(types::u8 reg);
  void write8(types::u8 reg, types::u8 data);

  PinInputControl inputControl;
  PinOutputControl outputControl;
  PinSelector pinSelector;

  spi_inst_t *spi_obj;
};