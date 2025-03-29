#include "ALSPT19.hpp"
#include "pinmap.hpp"
#include "pins/digital_pins.hpp"
#include "pins/types.hpp"
#include "types.hpp"
#include "comms.hpp"
#include "debug.hpp"

extern "C" {
#include "hardware/adc.h"
#include <pico/stdlib.h>
}

void LineSensor::init() {
  debug::debug("---> Initializing ALSPT19\r\n");

  //init dmux gpio pins
  pins::digital_pins.set_mode(pinmap::Digital::AMUX_S0, pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.set_mode(pinmap::Digital::AMUX_S1, pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.set_mode(pinmap::Digital::AMUX_S2, pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.set_mode(pinmap::Digital::AMUX_S3, pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.set_mode(pinmap::Digital::AMUX_EN, pins::DigitalPinMode::OUTPUT);
  pins::digital_pins.write(pinmap::Digital::AMUX_EN, 0);
  debug::debug("Line Sensors: Init DMUX GPIO\r\n");

  //init adc
  adc_init(); // initialise ADC
  adc_gpio_init((int)pinmap::Pico::AMUX1_COM);
  adc_gpio_init((int)pinmap::Pico::AMUX2_COM);
  adc_gpio_init((int)pinmap::Pico::AMUX3_COM);
}

void LineSensor::select_channel(uint8_t channel) {
  pins::digital_pins.write(pinmap::Digital::AMUX_S0, channel & 0x01);
  pins::digital_pins.write(pinmap::Digital::AMUX_S1, channel & 0x02);
  pins::digital_pins.write(pinmap::Digital::AMUX_S2, channel & 0x04);
  pins::digital_pins.write(pinmap::Digital::AMUX_S3, channel & 0x08);
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
