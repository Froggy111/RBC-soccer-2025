#pragma once
#include "types.hpp"
#include "pin_selector.hpp"
#include <hardware/spi.h>
#include <pico/types.h>
#include "pins/MCP23S17.hpp"

namespace mouse {
class MouseSensor {
public:
  bool init(int id, spi_inst_t *spi_obj_touse);

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

  Pins pins;
  spi_inst_t *spi_obj;

  int _id = 0;

  static MCP23S17 dmux1, dmux2;
  static bool dmux_init[2];
};
} // namespace mouse