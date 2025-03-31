#pragma once
#include "types.hpp"

namespace pinmap {

enum class Pico : types::u8 {
  DRV2_IN1 = 0,
  DRV2_SCS = 1,
  SPI0_SCLK = 2,
  SPI0_MOSI = 3,
  SPI0_MISO = 4,
  DMUX_SCS = 5,
  I2C1_SDA = 6,
  I2C1_SCL = 7,
  DRV3_IN1 = 8,
  DRV3_SCS = 9,
  DMUX2_INT = 10,
  MOUSE2_SCS = 11,
  DRV5_IN1 = 12,
  DRV5_SCS = 13,
  DMUX_RESET = 14,
  KICK = 15,
  UART0_TX = 16,
  UART0_RX = 17,
  DRV1_IN1 = 18,
  DRV1_SCS = 19,
  DRV4_IN1 = 20,
  DRV4_SCS = 21,
  MOUSE1_SCS = 22,
  AMUX3_COM = 26,
  AMUX2_COM = 27,
  AMUX1_COM = 28
};

enum class Mux1A : types::u8 {
  DRV4_OFF = 0,
  MOUSE1_RST = 1,
  MOUSE1_MOT = 2,
  AMUX_S3 = 3,
  AMUX_S2 = 4,
  AMUX_EN = 5,
  AMUX_S1 = 6,
  AMUX_S0 = 7
};

enum class Mux1B : types::u8 {
  DRV1_OFF = 0,
  DRV1_IN2 = 1,
  DRV1_NSLEEP = 2,
  DRV5_IN2 = 3,
  DRV5_OFF = 4,
  DRV5_NSLEEP = 5,
  DRV4_NSLEEP = 6,
  DRV4_IN2 = 7
};

enum class Mux2A : types::u8 {
  DRV2_NFAULT = 0,
  DRV2_NSLEEP = 1,
  MOUSE2_RST = 2,
  MOUSE2_MOT = 3,
  DRV3_IN2 = 4,
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
  DRV2_IN2 = 7
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
