#pragma once
#include "types.hpp"

namespace pinmap {

enum class DMUX1A : types::u8 {
  US_INT1 = 0,
  US_PROG1 = 1,
  US_INT2 = 2,
  US_PROG2 = 3,
  US_INT3 = 4,
  US_PROG3 = 5,
  US_INT4 = 6,
  US_PROG4 = 7
};

enum class DMUX1B : types::u8 {
  US_INT5 = 0,
  US_PROG5 = 1,
  US_INT6 = 2,
  US_PROG6 = 3,
  US_INT7 = 4,
  US_PROG7 = 5,
  US_INT8 = 6,
  US_PROG8 = 7
};

enum class DMUX2A : types::u8 {
  US_INT9 = 0,
  US_PROG9 = 1,
  US_INT10 = 2,
  US_PROG10 = 3,
  US_INT11 = 4,
  US_PROG11 = 5,
  US_INT12 = 6,
  US_PROG12 = 7
};

enum class DMUX2B : types::u8 {
  US_INT13 = 0,
  US_PROG13 = 1,
  US_INT14 = 2,
  US_PROG14 = 3,
  US_INT15 = 4,
  US_PROG15 = 5,
  US_INT16 = 6,
  US_PROG16 = 7
};

enum class Pico : types::u8 {
  LED_SIG_DIR = 0,
  LED_SIG_3V3 = 1,
  DMUX1_INTA = 4,
  US_NRST = 5,
  SPI0_MISO = 6,
  DMUX_SCS = 7,
  SPI0_SCLK = 9,
  SPI0_MOSI = 10,
  DMUX1_NRST = 11,
  IMU1_INT = 12,
  SPI1_SCLK = 14,
  SPI1_MOSI = 15,
  IMU1_NCS = 16,
  OLED_NRST = 17,
  OLED_D_C = 19,
  OLED_SCS = 20,
  I2C0_SCL = 21,
  I2C0_SDA = 20,
  MIC1_SD = 25,
  MIC1_LR = 24,
  MIC1_WS = 22,
  MIC1_SCK = 21,
  DMUX2_INTA = 31,
  DMUX2_NRST = 29,
  IMU2_NCS = 34,
  IMU2_INT = 32
};

} // namespace pinmap
