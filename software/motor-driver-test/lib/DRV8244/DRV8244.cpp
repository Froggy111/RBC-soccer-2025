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

/** PINS
 * Arduino CLK = PIN 13
 * Arduino SDI (MISO) = PIN 12
 * Arduino SDO (MOSI) = PIN 11
 */
#define CS_PIN 5
#define NSLEEP_PIN 15
#define NFAULT_PIN 4
#define IN1_PIN 16
#define DRVOFF_PIN 17
#define IN2_PIN 13
#define IPROPI_pin 0

// Constants for IPROPI current calculation
const uint16_t A_IPROPI = 4750; // VQFN-HR package
const uint32_t IPROPI_resistance = 680;

namespace DRV8244
{
  DRV8244::DRV8244(void)
  {
    this->_driver_mode = DriverMode::uncertain;
    this->_driver_state = DriverState::uncertain;
    return;
  };

  // * For initializing the DRV8244 driver
  bool DRV8244::start(GenericSPIClass SPI, DriverMode driver_mode, DriverState driver_state, bool display_active)
  {
    this->_SPI = SPI;
    this->_display_active = display_active;
    this->_driver_state = driver_state;
    this->_driver_mode = driver_mode;

    if (_display_active){
      _display_controller = Display();
    }
  }

  /*
   * This SPI function is used to write the set device configurations and operating
   * parameters of the device.
   Register format |R/W|A5|A4|A3|A2|A1|A0|*|D7|D6|D5|D4|D3|D2|D1|D0|
   Ax is address bit, Dx is data bits and R/W is read write bit.
   For write R/W bit should 0.
   */
  void DRV8244::write8(uint8_t reg, uint8_t value)
  {
    volatile uint16_t reg_value = 0;                            // Variable for the combined register and data info
    reg_value |= ((reg << SPI_ADDRESS_POS) & SPI_ADDRESS_MASK); // Adding register address value
    reg_value |= ((value << SPI_DATA_POS) & SPI_DATA_MASK);     // Adding data value

    digitalWrite(CS_PIN, LOW);

    SPI.transfer((uint8_t)((reg_value >> 8) & 0xFF));
    uint16_t recieved = SPI.transfer((uint8_t)(reg_value & 0xFF));
    print_screen("WRITE8 RECEIVED: " + std::to_string(recieved), false);

    digitalWrite(CS_PIN, HIGH);
  }

  void DRV8244::print_screen(std::string text, bool flush)
  {
    if (_display_active)
    {
      _display_controller.print_screen(text, flush);
    }
    else
    {
      Serial.println(text.c_str());
    }
  }

  // * Configuring the driver
  bool DRV8244::set_pin_modes()
  {
    print_screen("PIN MODES: Setting");

    // Set pin modes
    pinMode(NSLEEP_PIN, OUTPUT);
    digitalWrite(NSLEEP_PIN, 1);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, 1);
    pinMode(IN2_PIN, OUTPUT);
    digitalWrite(IN2_PIN, 0);
    pinMode(IN1_PIN, OUTPUT);
    digitalWrite(IN1_PIN, 0);
    pinMode(NFAULT_PIN, OUTPUT);
    digitalWrite(NFAULT_PIN, 0);
    pinMode(DRVOFF_PIN, OUTPUT);
    digitalWrite(DRVOFF_PIN, 0);

    print_screen("PIN MODES: Set");
    return true;
  }

  bool DRV8244::set_spi()
  {
    print_screen("SPI: Setting");

    // Set SPI
    SPI.begin(); // initialize the SPI library
    SPI.setDataMode(SPI_MODE1);

    print_screen("SPI: Set");
    return true;
  }

  bool DRV8244::clear_fault()
  {
    print_screen("CLEAR FAULT: Sending");

    write8(0x08, 0b10000000); // Send CLR_FLT command

    print_screen("CLEAR FAULT: Sent");
    return true;
  }

  bool DRV8244::change_mode(DriverMode mode)
  {
    if (mode == DriverMode::pwm)
    {
      print_screen("CHANGE MODE: Changing to PWM");
      write8(0x0C, 0b01000011);
      digitalWrite(DRVOFF_PIN, 0);
      print_screen("CHANGE MODE: Changed to PWM");
      return true;
    }
    else
    {
      return false;
    }
  }

  // * Functions when the driver is in PWM mode
  float DRV8244::read_current()
  {
    float voltage = ((float)analogReadMilliVolts(IPROPI_pin)) / 1000;
    float IPROPI_current = voltage / IPROPI_resistance;
    float current = IPROPI_current * A_IPROPI; // only one high side is recorded in PWM mode, so its correct
    return current;
  }

  void DRV8244::set_speed(uint8_t speed)
  {
    print_screen("SET SPEED: Setting speed to " + std::to_string(speed));
    analogWrite(IN2_PIN, speed);
    print_screen("SET SPEED: Set speed to " + std::to_string(speed));
  }
}