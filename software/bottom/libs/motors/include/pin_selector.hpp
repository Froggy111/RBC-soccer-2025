#pragma once

#include "pinmap.hpp"
#include "types.hpp"
extern "C" {
#include <pico/stdlib.h>
}

// driver pinmap for DRV8244
typedef enum DriverPinMap {
  CS,
  MOSI,
  MISO,
  SCK,
  NSLEEP,
  NFAULT,
  IPROPI,
  IN2,
  IN1,
  DRVOFF
} DriverPinMap;

enum class Driver1PinMap {
  CS = (types::u8)pinmap::DigitalPins::DRV1_SCS,
  MOSI = (types::u8)pinmap::DigitalPins::SPI0_MOSI,
  MISO = (types::u8)pinmap::DigitalPins::SPI0_MISO,
  SCK = (types::u8)pinmap::DigitalPins::SPI0_SCLK,
  NSLEEP = (types::u8)pinmap::DigitalPins::DRV1_NSLEEP,
  NFAULT = (types::u8)pinmap::DigitalPins::DRV1_NFAULT,
  IPROPI = (types::u8)pinmap::AnalogPins::DRV1_IPROPI,
  IN2 = (types::u8)pinmap::DigitalPins::DRV1_IN2,
  IN1 = (types::u8)pinmap::DigitalPins::DRV1_IN1,
  DRVOFF = (types::u8)pinmap::DigitalPins::DRV1_OFF
};

enum class Driver2PinMap {
  CS = (types::u8)pinmap::DigitalPins::DRV2_SCS,
  MOSI = (types::u8)pinmap::DigitalPins::SPI0_MOSI,
  MISO = (types::u8)pinmap::DigitalPins::SPI0_MISO,
  SCK = (types::u8)pinmap::DigitalPins::SPI0_SCLK,
  NSLEEP = (types::u8)pinmap::DigitalPins::DRV2_NSLEEP,
  NFAULT = (types::u8)pinmap::DigitalPins::DRV2_NFAULT,
  IPROPI = (types::u8)pinmap::AnalogPins::DRV2_IPROPI,
  IN2 = (types::u8)pinmap::DigitalPins::DRV2_IN2,
  IN1 = (types::u8)pinmap::DigitalPins::DRV2_IN1,
  DRVOFF = (types::u8)pinmap::DigitalPins::DRV2_OFF
};

enum class Driver3PinMap {
  CS = (types::u8)pinmap::DigitalPins::DRV3_SCS,
  MOSI = (types::u8)pinmap::DigitalPins::SPI0_MOSI,
  MISO = (types::u8)pinmap::DigitalPins::SPI0_MISO,
  SCK = (types::u8)pinmap::DigitalPins::SPI0_SCLK,
  NSLEEP = (types::u8)pinmap::DigitalPins::DRV3_NSLEEP,
  NFAULT = (types::u8)pinmap::DigitalPins::DRV3_NFAULT,
  IPROPI = (types::u8)pinmap::AnalogPins::DRV3_IPROPI,
  IN2 = (types::u8)pinmap::DigitalPins::DRV3_IN2,
  IN1 = (types::u8)pinmap::DigitalPins::DRV3_IN1,
  DRVOFF = (types::u8)pinmap::DigitalPins::DRV3_OFF
};

enum class Driver4PinMap {
  CS = (types::u8)pinmap::DigitalPins::DRV4_SCS,
  MOSI = (types::u8)pinmap::DigitalPins::SPI0_MOSI,
  MISO = (types::u8)pinmap::DigitalPins::SPI0_MISO,
  SCK = (types::u8)pinmap::DigitalPins::SPI0_SCLK,
  NSLEEP = (types::u8)pinmap::DigitalPins::DRV4_NSLEEP,
  NFAULT = (types::u8)pinmap::DigitalPins::DRV4_NFAULT,
  IPROPI = (types::u8)pinmap::AnalogPins::DRV4_IPROPI,
  IN2 = (types::u8)pinmap::DigitalPins::DRV4_IN2,
  IN1 = (types::u8)pinmap::DigitalPins::DRV4_IN1,
  DRVOFF = (types::u8)pinmap::DigitalPins::DRV4_OFF
};

enum class DriverDbgPinMap {
  CS = 1,
  MOSI = 3,
  MISO = 0,
  SCK = 2,
  NSLEEP = 9,
  NFAULT = 7,
  IPROPI = 8,
  IN2 = 5,
  IN1 = 4,
  DRVOFF = 6
};

class PinSelector {
  public:
    types::u8 get_pin(DriverPinMap pin);
    void set_driver_id(types::u8 id);
    void set_debug_mode(bool mode);

  private:
    types::u8 driverId;
    bool debugMode;
};