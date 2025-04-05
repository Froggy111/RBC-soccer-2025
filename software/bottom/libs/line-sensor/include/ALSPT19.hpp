#pragma once
#include "types.hpp"
#include "pins/MCP23S17.hpp"

class LineSensor {
public:
  void init(spi_inst_t *spi_obj);
  uint16_t read_raw(uint8_t line_sensor_id);

private:
  void select_channel(uint8_t channel);
  MCP23S17 dmux;
  types::u8 current_channel = 255;
};
