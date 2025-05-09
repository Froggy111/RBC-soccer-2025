#include "pin_manager.hpp"
#include "pins/ADS1115.hpp"
#include "pins/MCP23S17.hpp"
#include "pin_selector.hpp"
#include "comms.hpp"
#include "debug.hpp"
extern "C" {
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/i2c.h>
}

#define ADC1_ADDR 0b1001000
#define ADC2_ADDR 0b1001001
#define ADC_CLK_SPEED 300 // 0.3 MHz, both fast and high speed modes
#define ADC_DATA_RATE 28

namespace driver {
//! Input Control
void PinOutputControl::init(bool dbg, spi_inst_t *spi_obj) {
  debug = dbg;
  if (!debug) {
    dmux1.init(1, spi_obj);
    dmux2.init(2, spi_obj);
  }
}

// * Digital Pins

// pins responsible for providing input to DRV8244
void PinOutputControl::init_digital(types::u8 pin, bool value,
                                    PinInterface interface) {
  if (debug || interface == GPIO) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
  } else {
    if (interface == MUX1A || interface == MUX1B)
      dmux1.init_gpio(pin, interface == MUX1A, true);
    else
      dmux2.init_gpio(pin, interface == MUX2A, true);
  }

  this->digital_cache[pin] = value;
  write_digital(pin, value, interface);
}

void PinOutputControl::write_digital(types::u8 pin, bool value,
                                     PinInterface interface) {
  if (this->digital_cache.find(pin) == this->digital_cache.end()) {
    debug::log("Pin not initialized! pin %d\r\n", pin);
    return;
  }

  if (debug || interface == GPIO) {
    gpio_put(pin, value);
  } else {
    if (interface == MUX1A || interface == MUX1B)
      dmux1.write_gpio(pin, interface == MUX1A, value);
    else
      dmux2.write_gpio(pin, interface == MUX2A, value);
  }

  // debug::log("%d has been written to pin %d\n", value, pin);
  this->digital_cache[pin] = value;
}

bool PinOutputControl::get_last_value_digital(types::u8 pin) {
  if (this->digital_cache.find(pin) == this->digital_cache.end()) {
    debug::log("Pin not initialized! pin %d\r\n", pin);
    // TODO: Fix return value
    return false;
  }

  return this->digital_cache[pin];
}

//* PWM

void PinOutputControl::init_pwm(types::u8 pin, int value) {
  gpio_set_function(pin, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(pin);
  uint channel = pwm_gpio_to_channel(pin);

  // Set PWM configuration
  pwm_set_clkdiv(slice_num, 4.0);
  pwm_set_wrap(slice_num, 12500); // set max value

  // Set PWM level
  this->pwm_cache[pin] = value;
  write_pwm(pin, value);

  // enable PWM
  pwm_set_enabled(slice_num, true);
}

void PinOutputControl::write_pwm(types::u8 pin, int value) {
  if (this->pwm_cache.find(pin) == this->pwm_cache.end()) {
    debug::log("Pin not initialized! pin %d\r\n", pin);
    return;
  }
  uint slice_num = pwm_gpio_to_slice_num(pin);
  uint channel = pwm_gpio_to_channel(pin);
  // debug::log("%d has been written to pin %d\n", value, pin);
  pwm_set_chan_level(slice_num, channel, value);
}

//! Output Control

void PinInputControl::init(bool dbg, spi_inst_t *spi_obj) {
  debug = dbg;
  if (!debug) {
    dmux1.init(1, spi_obj);
    dmux2.init(2, spi_obj);

    if (!adc_init[0] &&
        !adc1.beginADSX((PICO_ADS1115::ADSXAddressI2C_e)ADC1_ADDR, i2c1,
                        ADC_CLK_SPEED, (uint8_t)pinmap::Pico::I2C1_SDA,
                        (uint8_t)pinmap::Pico::I2C1_SCL, 1000)) {
      debug::log("ADC1 not found!\r\n");
    } else {
      adc_init[0] = true;
      adc1.setGain(PICO_ADS1115::ADSXGain_TWO);
      adc1.setDataRate(ADC_DATA_RATE);
    }

    if (!adc_init[1] &&
        !adc2.beginADSX((PICO_ADS1115::ADSXAddressI2C_e)ADC2_ADDR, i2c1,
                        ADC_CLK_SPEED, (uint8_t)pinmap::Pico::I2C1_SDA,
                        (uint8_t)pinmap::Pico::I2C1_SCL, 1000)) {
      debug::log("ADC2 not found!\r\n");
    } else {
      adc_init[1] = true;
      adc2.setGain(PICO_ADS1115::ADSXGain_TWO);
      adc2.setDataRate(ADC_DATA_RATE);
    }
  }
}

// * Digital Pins
void PinInputControl::init_digital(types::u8 pin, PinInterface interface) {
  if (debug) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
  } else {
    if (interface == MUX1A || interface == MUX1B)
      dmux1.init_gpio(pin, interface == MUX1A, false);
    else
      dmux2.init_gpio(pin, interface == MUX2A, false);
  }
}

void PinInputControl::pullup_digital(types::u8 pin, PinInterface interface) {
  if (debug) {
    gpio_pull_up(pin);
  } else {
    if (interface == MUX1A || interface == MUX1B)
      dmux1.pullup_gpio(pin, interface == MUX1A);
    else
      dmux2.pullup_gpio(pin, interface == MUX2A);
  }
}

bool PinInputControl::read_digital(types::u8 pin, PinInterface interface) {
  if (debug) {
    bool result = gpio_get(pin);
    // debug::log("%d has been read from pin %d\n", result, pin);
    return result;
  } else {
    if (interface == MUX1A || interface == MUX1B)
      return dmux1.read_gpio(pin, interface == MUX1A);
    else
      return dmux2.read_gpio(pin, interface == MUX2A);
  }
}

// * Analog Pins

int16_t PinInputControl::read_analog(types::u8 pin, PinInterface interface) {
  if (debug) {
    // TODO
    return 0;
  } else {
    if (interface == ADC1)
      return adc1.readADC_SingleEnded((PICO_ADS1X15::ADSX_AINX_e)pin);
    else
      return adc2.readADC_SingleEnded((PICO_ADS1X15::ADSX_AINX_e)pin);
  }
}

} // namespace driver