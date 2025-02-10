#include "libs/utils/types.hpp"

namespace pinmap {

enum class Pins : types::u8 {
  DRV2_IN1 = 0,
  DRV2_IN2 = 1,
  SPI0_SCLK = 2,
  SPI0_MOSI = 3,
  SPI0_MISO = 4,
  DMUX_SCS = 5,
  I2C0_SDA = 6,
  I2C0_SCL = 7,
  DRV3_IN1 = 8,
  DRV3_IN2 = 9,
  DMUX2_INT = 10,
  MOUSE2_MOT = 11,
  DRV5_IN1 = 12,
  DRV5_IN2 = 13,
  DMUX_RESET = 14,
  KICK = 15,
  UART0_TX = 16,
  UART0_RX = 17,
  DRV1_IN1 = 18,
  DRV1_IN2 = 19,
  DRV4_IN1 = 20,
  DRV4_IN2 = 21,
  MOUSE1_MOT = 22,
  AMUX3_COM = 26,
  AMUX2_COM = 27,
  AMUX1_COM = 28,

  // Mux1A
  DRV4_OFF = 29,
  MOUSE1_RST = 30,
  MOUSE1_SCS = 31,
  AMUX_S3 = 32,
  AMUX_S2 = 33,
  AMUX_EN = 34,
  AMUX_S1 = 35,
  AMUX_S0 = 36,

  // Mux1B
  DRV1_OFF = 37,
  DRV1_SCS = 38,
  DRV1_NSLEEP = 39,
  DRV5_SCS = 40,
  DRV5_OFF = 41,
  DRV5_NSLEEP = 42,
  DRV4_NSLEEP = 43,
  DRV4_SCS = 44,

  // Mux2A
  DRV2_NFAULT = 45,
  DRV2_NSLEEP = 46,
  MOUSE2_RST = 47,
  MOUSE2_SCS = 48,
  DRV3_SCS = 49,
  DRV3_OFF = 50,
  DRV3_NSLEEP = 51,
  DRV3_NFAULT = 52,

  // Mux2B
  DRV5_NFAULT = 53,
  ADC1_INT = 54,
  ADC2_INT = 55,
  DRV1_NFAULT = 56,
  DRV4_NFAULT = 57,
  DMUX1_INT = 58,
  DRV2_OFF = 59,
  DRV2_SCS = 60,

  // ADC1
  DRV2_IPROPI = 61,
  LIGHTGATE = 62,
  DRV3_IPROPI = 63,
  DRV5_IPROPI = 64,

  // ADC2
  LOADCELL_AMINUS = 65,
  LOADCELL_APLUS = 66,
  DRV1_IPROPI = 67,
  DRV4_IPROPI = 68
};

enum class Pico : types::u8 {
  DRV2_IN1 = 0,
  DRV2_IN2 = 1,
  SPI0_SCLK = 2,
  SPI0_MOSI = 3,
  SPI0_MISO = 4,
  DMUX_SCS = 5,
  I2C0_SDA = 6,
  I2C0_SCL = 7,
  DRV3_IN1 = 8,
  DRV3_IN2 = 9,
  DMUX2_INT = 10,
  MOUSE2_MOT = 11,
  DRV5_IN1 = 12,
  DRV5_IN2 = 13,
  DMUX_RESET = 14,
  KICK = 15,
  UART0_TX = 16,
  UART0_RX = 17,
  DRV1_IN1 = 18,
  DRV1_IN2 = 19,
  DRV4_IN1 = 20,
  DRV4_IN2 = 21,
  MOUSE1_MOT = 22,
  AMUX3_COM = 26,
  AMUX2_COM = 27,
  AMUX1_COM = 28
};

enum class Mux1A : types::u8 {
  DRV4_OFF = 0,
  MOUSE1_RST = 1,
  MOUSE1_SCS = 2,
  AMUX_S3 = 3,
  AMUX_S2 = 4,
  AMUX_EN = 5,
  AMUX_S1 = 6,
  AMUX_S0 = 7
};

enum class Mux1B : types::u8 {
  DRV1_OFF = 0,
  DRV1_SCS = 1,
  DRV1_NSLEEP = 2,
  DRV5_SCS = 3,
  DRV5_OFF = 4,
  DRV5_NSLEEP = 5,
  DRV4_NSLEEP = 6,
  DRV4_SCS = 7
};

enum class Mux2A : types::u8 {
  DRV2_NFAULT = 0,
  DRV2_NSLEEP = 1,
  MOUSE2_RST = 2,
  MOUSE2_SCS = 3,
  DRV3_SCS = 4,
  DRV3_OFF = 5,
  DRV3_NSLEEP = 6,
  DRV3_NFAULT = 7
};

enum class Mux2B : types::u8 {
  DRV5_NFAULT = 0,
  ADC1_INT = 1,
  ADC2_INT = 2,
  DRV1_NFAULT = 3,
  DRV4_NFAULT = 4,
  DMUX1_INT = 5,
  DRV2_OFF = 6,
  DRV2_SCS = 7
};

enum class ADC1 : types::u8 {
  DRV2_IPROPI = 0,
  LIGHTGATE = 1,
  DRV3_IPROPI = 2,
  DRV5_IPROPI = 3
};

enum class ADC2 : types::u8 {
  LOADCELL_AMINUS = 0,
  LOADCELL_APLUS = 1,
  DRV1_IPROPI = 2,
  DRV4_IPROPI = 3
};

} // namespace pinmap
