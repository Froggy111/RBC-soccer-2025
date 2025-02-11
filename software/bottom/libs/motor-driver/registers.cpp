#include "pico/stdlib.h"
#include "pinmap.hpp"
#include "hardware/spi.h"

class Registers {
	static uint8_t readRegister(uint8_t reg)
	{
	  uint8_t txBuf[2];
	  uint8_t rxBuf[2];
	  // If your protocol requires a read bit, you might do:
	  // txBuf[0] = reg | 0x80;
	  txBuf[0] = reg;
	  txBuf[1] = 0x00; // dummy byte
  
	  // Assert chip-select (active low)
	  gpio_put(PinMap::CS, 0);
	  spi_write_read_blocking(spi0, txBuf, rxBuf, 2);
	  // Deassert chip-select
	  gpio_put(PinMap::CS, 1);
  
	  // Return the second byte received (the register data)
	  return rxBuf[1];
	}

	static void writeRegister(uint8_t reg, uint8_t data)
	{
	  uint8_t txBuf[2];
	  // If your protocol requires a write bit, you might do:
	  // txBuf[0] = reg & 0x7F;
	  txBuf[0] = reg;
	  txBuf[1] = data;
  
	  // Assert chip-select (active low)
	  gpio_put(PinMap::CS, 0);
	  spi_write_blocking(spi0, txBuf, 2);
	  // Deassert chip-select
	  gpio_put(PinMap::CS, 1);
	}
};