#pragma once

#include "pinmap.hpp"
#include "types.hpp"
#include <hardware/spi.h>
#include <pico/stdlib.h>

class MCP23S17 {
private:
  types::u8 id;
  types::u8 pin_state[17];
  spi_inst_t *spi_obj;

  static bool initialized[2];

  /**
   * @brief Init the fSPI interface. Calls configure_spi and sets any registers it needs to.
   * 
   */
  void init_spi();

  /**
   * @brief Init the GPIO pins.
   * 
   */
  void init_pins();

  /**
   * @brief Configure the SPI interface.
   * 
   */
  void configure_spi();

  /**
   * @brief Read a byte from a register.
   * 
   * @param device_address 
   * @param reg_address 
   * @return uint8_t 
   */
  uint8_t read8(uint8_t device_address, uint8_t reg_address);

  /**
   * @brief Write a byte to a register, optionally providing a mask to only write a few bits.
   * 
   * @param device_address 
   * @param reg_address 
   * @param data 
   * @param mask 
   */
  void write8(uint8_t device_address, uint8_t reg_address, uint8_t data, uint8_t mask = 0xFF);

public:
  /**
   * @brief Initialize the MCP23S17.
   * 
   * @param device_id 
   * @param spi_obj 
   */
  void init(types::u8 device_id, spi_inst_t *spi_obj);

  /**
   * @brief Reset the device.
   * 
   */
  void reset();

  /**
   * @brief Initialize a GPIO pin. This is **to be called before read/write_gpio**.
   * if "output" is set to true, the MCP23S17 will write to the pin, otherwise it will read from the pin
   *
   * @param pin 
   * @param on_A 
   * @param is_output 
   */
  void init_gpio(uint8_t pin, bool on_A, bool is_output);
  /**
   * @brief Write to a GPIO pin.
   * 
   * @param pin 
   * @param on_A 
   * @param value 
   */
  void write_gpio(uint8_t pin, bool on_A, bool value);

  /**
   * @brief Read a GPIO pin.
   * 
   * @param pin 
   * @param on_A 
   */
  bool read_gpio(uint8_t pin, bool on_A);

  /**
   * @brief Pullup a GPIO pin.
   * 
   * @param pin 
   * @param on_A 
   */
   void pullup_gpio(uint8_t pin, bool on_A);
};