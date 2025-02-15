#pragma once
#include "types.hpp"

namespace pinmap {

enum class DigitalPins : types::u8 {
  // Pico
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

  // Mux1A
  DRV4_OFF = 23,
  MOUSE1_RST = 24,
  MOUSE1_SCS = 25,
  AMUX_S3 = 26,
  AMUX_S2 = 27,
  AMUX_EN = 28,
  AMUX_S1 = 29,
  AMUX_S0 = 30,

  // Mux1B
  DRV1_OFF = 31,
  DRV1_SCS = 32,
  DRV1_NSLEEP = 33,
  DRV5_SCS = 34,
  DRV5_OFF = 35,
  DRV5_NSLEEP = 36,
  DRV4_NSLEEP = 37,
  DRV4_SCS = 38,

  // Mux2A
  DRV2_NFAULT = 39,
  DRV2_NSLEEP = 40,
  MOUSE2_RST = 41,
  MOUSE2_SCS = 42,
  DRV3_SCS = 43,
  DRV3_OFF = 44,
  DRV3_NSLEEP = 45,
  DRV3_NFAULT = 46,

  // Mux2B
  DRV5_NFAULT = 47,
  ADC1_INT = 48,
  ADC2_INT = 49,
  DRV1_NFAULT = 50,
  DRV4_NFAULT = 51,
  DMUX1_INT = 52,
  DRV2_OFF = 53,
  DRV2_SCS = 54,
};

enum class AnalogPins : types::u8 {
  // Pico
  AMUX3_COM = 26,
  AMUX2_COM = 27,
  AMUX1_COM = 28,

  // ADC1
  DRV2_IPROPI = 29,
  LIGHTGATE = 30,
  DRV3_IPROPI = 31,
  DRV5_IPROPI = 32,

  // ADC2
  LOADCELL_AMINUS = 33,
  LOADCELL_APLUS = 34,
  DRV1_IPROPI = 35,
  DRV4_IPROPI = 36,

  // AMUX1
};

// NOTE: The below are mostly for reference. Pin numbers can be obtained from subtracting values depending on range.
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

enum class AMUX1 : types::u8 {
  LS1 = 0,
  LS2 = 1,
  LS3 = 2,
  LS4 = 3,
  LS5 = 4,
  LS6 = 5,
  LS7 = 6,
  LS8 = 7,
  LS16 = 8,
  LS15 = 9,
  LS14 = 10,
  LS13 = 11,
  LS12 = 12,
  LS11 = 13,
  LS10 = 14,
  LS9 = 15
};

enum class AMUX2 : types::u8 {
  LS28 = 0,
  LS27 = 1,
  LS26 = 2,
  LS25 = 3,
  LS24 = 4,
  LS23 = 5,
  LS22 = 6,
  LS21 = 7,
  LS17 = 8,
  LS18 = 9,
  LS19 = 10,
  LS20 = 11,
  LS29 = 12,
  LS30 = 13,
  LS31 = 14,
  LS32 = 15
};

enum class AMUX3 : types::u8 {
  LS40 = 0,
  LS39 = 1,
  LS38 = 2,
  LS37 = 3,
  LS36 = 4,
  LS35 = 5,
  LS34 = 6,
  LS33 = 7,
  LS41 = 8,
  LS42 = 9,
  LS43 = 10,
  LS44 = 11,
  LS45 = 12,
  LS46 = 13,
  LS47 = 14,
  LS48 = 15
};

} // namespace pinmap
