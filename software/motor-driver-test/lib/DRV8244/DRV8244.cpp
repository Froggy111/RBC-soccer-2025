#include <Arduino.h>
#include <SPI.h>
#include "typedefs.hpp"
#include "DRV8244.hpp"

// SPI Protocol
#define SPI_ADDRESS_MASK 0x3F00 // Mask for SPI register address bits
#define SPI_ADDRESS_POS 8       // Position for SPI register address bits
#define SPI_DATA_MASK 0x00FF    // Mask for SPI register data bits
#define SPI_DATA_POS 0          // Position for SPI register data bits
#define SPI_RW_BIT_MASK 0x4000  // Mask for SPI register read write indication bit
#define CS_PIN 5

#define IN2_PIN 13
const uint16_t A_IPROPI = 4750; // VQFN-HR package
const uint32_t IPROPI_resistance = 680;
const uint32_t ipropi_moving_average_size = 16384; // change this if uw, if its too big it wont fit
const uint8_t IPROPI_pin = 0;                      // @jx put the IPROPI pin number here, also make sure its a pin that can read analog

namespace DRV8244
{
  DRV8244::DRV8244(void)
  {
    this->_driver_mode = DriverMode::uncertain;
    this->_driver_state = DriverState::uncertain;
    return;
  };

  bool DRV8244::begin(GenericSPIClass SPI, DriverMode driver_mode, DriverState driver_state)
  {
    if (!this->set_spi(SPI))
    {
      return false;
    }
    if (this->set_driver_mode(driver_mode))
    {
      return false;
    }
    if (this->set_driver_state(driver_state))
    {
      return false;
    }
  }

  bool DRV8244::set_spi(GenericSPIClass SPI)
  {
    this->_SPI = SPI;
    return true;
  }

  /**
   * This SPI function is used to write the set device configurations and operating
   * parameters of the device.
   * Register format |R/W|A5|A4|A3|A2|A1|A0|*|D7|D6|D5|D4|D3|D2|D1|D0|
   * Ax is address bit, Dx is data bits and R/W is read write bit.
   * For write R/W bit should 0.
   */
  void DRV8244::write8(uint8_t reg, uint8_t value)
  {
    volatile uint16_t reg_value = 0;                            // Variable for the combined register and data info
    reg_value |= ((reg << SPI_ADDRESS_POS) & SPI_ADDRESS_MASK); // Adding register address value
    reg_value |= ((value << SPI_DATA_POS) & SPI_DATA_MASK);     // Adding data value

    digitalWrite(CS_PIN, LOW);

    SPI.transfer((uint8_t)((reg_value >> 8) & 0xFF));
    uint16_t recieved = SPI.transfer((uint8_t)(reg_value & 0xFF));
    // display_controller.print_screen_hex((uint8_t)(recieved), false);
    // display_controller.print_screen_hex((uint8_t)(recieved >> 8), false);

    digitalWrite(CS_PIN, HIGH);
  }

  bool DRV8244::set_driver_mode(DriverMode driver_mode)
  {
    /**
     * Set the S_MODE register.
     * PH/EN mode: 2'b00
     * Independent mode: 2'b01
     * PWM mode: 2'b10 / 2'b11
     */
  }

  float DRV8244::read_current()
  {
    float voltage = ((float)analogReadMilliVolts(IPROPI_pin)) / 1000;
    float IPROPI_current = voltage / IPROPI_resistance;
    float current = IPROPI_current * A_IPROPI; // only one high side is recorded in PWM mode, so its correct
    return current;
  }

  void DRV8244::set_speed(uint8_t speed)
  {
    analogWrite(IN2_PIN, speed);
  }
}