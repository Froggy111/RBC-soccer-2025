#pragma once
#include "registers.hpp"
#include "types.hpp"
#include "pin_selector.hpp"
#include <hardware/spi.h>
#include <pico/types.h>
#include "pins/ADS1115.hpp"

namespace driver {

class MotorDriver {
private:
  /**
   * @brief Initialize all SPI pins and set SPI interface modes
   * 
   */
  void configure_spi();

  /**
   * @brief Initialize all GPIO and analog pins
   * 
   */
  void init_pins();

  // * register reading

  /**
   * @brief Read 8 bits from a register on the DRV8244
   * 
   * @param reg 
   * @return types::u8 
   */
  types::u8 read8(types::u8 reg);

  /**
   * If expected is -1, then it will ensure that the value returned by the DRV8244 via SPI is the same as what is written.
   * Else, it will check if the value returned by the DRV8244 via SPI is the same as the expected value.
   * @brief Write 8 bits to a register on the DRV8244
   * 
   * @param reg 
   * @param data 
   * @param expected 
   * @return true 
   * @return false 
   */
  bool write8(types::u8 reg, types::u8 data, int8_t expected = -1);

  // * register handling

  /**
   * @brief Initialize the registers to their default values
   * 
   * @return true 
   * @return false 
   */
  bool init_registers();

  /**
   * @brief Check the registers to see if they are configured correctly to command the motor
   * 
   * @return true 
   * @return false 
   */
  bool check_registers(); // check if the registers are configured correctly

  /**
   * @brief Read the FAULT_SUMMARY register, and return a string of the faults
   * 
   * @return std::string 
   */
  std::string read_fault_summary();

  /**
   * @brief Read the STATUS1 register, and return a string of the status
   * 
   * @return std::string 
   */
  std::string read_status1();

  /**
   * @brief Read the STATUS2 register, and return a string of the status
   * 
   * @return std::string 
   */
  std::string read_status2();

  //* others

  /**
   * @brief Check that the pins are configured correctly
   * 
   * @return true 
   * @return false 
   */
  bool check_config();

  Pins pins;
  int _id;
  spi_inst_t *spi_obj;

  bool adc_init[2];
  PICO_ADS1115 adc1;
  PICO_ADS1115 adc2;

public:
  /**
   * @brief Init the GPIO & Analog Pins, the SPI interface and Registers
   * 
   * @param id 
   * @param SPI_SPEED 
   */
  void init(int id, spi_inst_t *spi_obj_touse);

  /**
   * TRUE = Sleeping, FALSE = Not Sleeping
   * @brief Set the NSLEEP pin, causing the motor to change between Standby and Active modes.
   * 
   * @param sleep 
   */
  void set_sleep(bool sleep);

  /**
   * Checks are implemented to ensure that the driver's registers, pins are all in the right state.
   * TODO: Implement Acceleration and Deceleration Safeguards.
   * @brief Command the motor with a duty cycle. 
   * 
   * @param duty_cycle Ranges from -12500 to 12500, Negative For Backwards, Positive for Forwards. 
   * @return true 
   * @return false 
   */
  bool command(types::i16 duty_cycle);

  /**
   * @brief Callback function to handle errors, to be called when NFAULT pin is high
   * 
   * @param driver 
   */
  static void handle_error(MotorDriver *driver);

  /**
   * @brief Returns the current running through the motor
   * 
   * @return int16_t 
   */
  int16_t read_current();

  /**
   * @brief Set the ITRIP register to the current_limit at which the motor should regulate
   * 
   * @param current_limit 
   */
  bool set_ITRIP(ITRIP::ITRIP current_limit);

  /**
   * @brief Set the OCP register to the current limit at which the motor should trip
   * 
   * @param current_limit 
   */
  bool set_OCP(OCP::OCP current_limit);
};

}