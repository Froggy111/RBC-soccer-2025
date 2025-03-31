#pragma once
#include "types.hpp"

namespace pinmap {

enum class PI : types::u8 {
  PI_I2C_SDA = 2,  // GP2
  PI_I2C_SCL = 3,  // GP3
  TOPPICO_TOGGLE_RUN = 4,  // GP4
  TOPPICO_TOGGLE_EN = 5,  // GP5
  BOTTOMPICO_TOGGLE_RUN = 6,  // GP6
  BOTTOMPICO_TOGGLE_EN13 = 7,  // GP7
  PI_UART_TX = 14,  // GP14
  PI_UART_RX = 15,  // GP15
  PCM_CLK = 18,  // GP18
  GP27 = 27,  // GP27
  RPI_SPI0_MOSI = 10,  // GP10
  RPI_SPI0_MISO = 9,  // GP9
  RPI_SPI0_SCLK = 11,  // GP11
  RPI_SPI0_SCS0 = 8,  // GP8
  RPI_SPI0_SCS1 = 7,  // GP7
  RPI_SPI0_SCS = 24,  // GP24
  MIDPICO_TOGGLE_EN = 20,  // GP20
  MIDPICO_TOGGLE_RUN = 28,  // GP28
  PI_SW1 = 31,  // GP31
  PI_SW2 = 29,  // GP29
  PI_SW3 = 27,  // GP27
  PI_SW4 = 34,  // GP34
  PI_SW5 = 37,  // GP37
  PWM0 = 12,  // GP12
  PWM1 = 13,  // GP13
  PCM_FS = 19,  // GP19
  PCM_DIN = 20,  // GP20
  PCM_DOUT = 21  // GP21
};

} 