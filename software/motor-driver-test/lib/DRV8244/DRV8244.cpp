#include <Arduino.h>
#include <SPI.h>
#include "typedefs.hpp"
#include "DRV8244.hpp"

namespace DRV8244 {
  DRV8244::DRV8244(void) {
    this->_driver_mode = DriverMode::uncertain;
    this->_driver_state = DriverState::uncertain;
    return;
  };

  bool DRV8244::begin(GenericSPIClass SPI, DriverMode driver_mode, DriverState driver_state) {
    if (!this->set_spi(SPI)) {
      return false;
    }
    if (this->set_driver_mode(driver_mode)) {
      return false;
    }
    if (this->set_driver_state(driver_state)) {
      return false;
    }
  }

  bool DRV8244::set_spi(GenericSPIClass SPI) {
    this->_SPI = SPI;
    return true;
  }

  bool DRV8244::set_driver_mode(DriverMode driver_mode) {
    /**
     * Set the S_MODE register.
     * PH/EN mode: 2'b00
     * Independent mode: 2'b01
     * PWM mode: 2'b10 / 2'b11
     */
  }
}