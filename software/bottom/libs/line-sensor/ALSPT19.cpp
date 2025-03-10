#include "ALSPT19.hpp"
#include "pins/MCP23S17.hpp"
#include "pinmap.hpp"
#include "types.hpp"
#include <cstdint>
#include "comms.hpp"

extern "C" {
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include <pico/stdlib.h>
}

void LineSensor::init(types::u8 id, spi_inst_t *spi_obj) {
  comms::USB_CDC.printf("---> Initializing ALSPT19\r\n");

  //init dmux
  dmux.init(1, spi_obj);
  comms::USB_CDC.printf("inited dmux\r\n");

  //init dmux gpio pins
  dmux.init_gpio((int)pinmap::Mux1A::AMUX_S0, true, true);
  dmux.init_gpio((int)pinmap::Mux1A::AMUX_S1, true, true);
  dmux.init_gpio((int)pinmap::Mux1A::AMUX_S2, true, true);
  dmux.init_gpio((int)pinmap::Mux1A::AMUX_S3, true, true);
  dmux.init_gpio((int)pinmap::Mux1A::AMUX_EN, true, true);
  dmux.write_gpio((int)pinmap::Mux1A::AMUX_EN, true, 0);
  comms::USB_CDC.printf("init dmux gpio\r\n");

  //init adc
  adc_init(); // initialise ADC
  adc_gpio_init((int)pinmap::Pico::AMUX1_COM);
  adc_gpio_init((int)pinmap::Pico::AMUX2_COM);
  adc_gpio_init((int)pinmap::Pico::AMUX3_COM);
  comms::USB_CDC.printf("done intialising\r\n");
}

void LineSensor::select_channel(uint8_t channel) {
  dmux.write_gpio((int)pinmap::Mux1A::AMUX_S0, true, channel & 0x01);
  dmux.write_gpio((int)pinmap::Mux1A::AMUX_S1, true, channel & 0x02);
  dmux.write_gpio((int)pinmap::Mux1A::AMUX_S2, true, channel & 0x04);
  dmux.write_gpio((int)pinmap::Mux1A::AMUX_S3, true, channel & 0x08);
}

// id starts from 0 to 47
uint16_t LineSensor::read_raw(uint8_t line_sensor_id) {
  if (line_sensor_id > 47) {
    comms::USB_CDC.printf("Invalid line sensor ID\n");
    return 0;
  }

  if (line_sensor_id < 16) {
    select_channel(line_sensor_id);
    adc_select_input(2);
    return adc_read();
  } else if (line_sensor_id < 32) {
    select_channel(line_sensor_id - 16);
    adc_select_input(1);
    return adc_read();
  } else {
    select_channel(line_sensor_id - 32);
    adc_select_input(0);
    return adc_read();
  }
}

float LineSensor::read_voltage() {
  // TODO: Implement this function
  return 0;
}
