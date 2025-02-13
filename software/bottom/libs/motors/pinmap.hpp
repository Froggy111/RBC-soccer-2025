#include "libs/hardware-descriptors/pinmap.hpp"
#include "libs/utils/types.hpp"
#include <map>
#include <string>

typedef std::map<std::string, types::u8> DriverPinMap;

// for debug pins, use -1 as driver_id
inline DriverPinMap generate_pinmap(int driver_id) {
  switch (driver_id) {
  case -1:
    return {{"CS", 21},    {"MOSI", 19},  {"MISO", 20},   {"SCK", 18},
            {"NSLEEP", 9}, {"NFAULT", 7}, {"IPROPI", 29}, {"IN2", 5},
            {"IN1", 4},    {"DRVOFF", 6}};
  case 1:
    return {{"CS", (types::u8)pinmap::DigitalPins::DRV1_SCS},
            {"MOSI", (types::u8)pinmap::DigitalPins::SPI0_MOSI},
            {"MISO", (types::u8)pinmap::DigitalPins::SPI0_MISO},
            {"SCK", (types::u8)pinmap::DigitalPins::SPI0_SCLK},
            {"NSLEEP", (types::u8)pinmap::DigitalPins::DRV1_NSLEEP},
            {"NFAULT", (types::u8)pinmap::DigitalPins::DRV1_NFAULT},
            {"IPROPI", (types::u8)pinmap::AnalogPins::DRV1_IPROPI},
            {"IN2", (types::u8)pinmap::DigitalPins::DRV1_IN2},
            {"IN1", (types::u8)pinmap::DigitalPins::DRV1_IN1},
            {"DRVOFF", (types::u8)pinmap::DigitalPins::DRV1_OFF}};
  case 2:
    return {{"CS", (types::u8)pinmap::DigitalPins::DRV2_SCS},
            {"MOSI", (types::u8)pinmap::DigitalPins::SPI0_MOSI},
            {"MISO", (types::u8)pinmap::DigitalPins::SPI0_MISO},
            {"SCK", (types::u8)pinmap::DigitalPins::SPI0_SCLK},
            {"NSLEEP", (types::u8)pinmap::DigitalPins::DRV2_NSLEEP},
            {"NFAULT", (types::u8)pinmap::DigitalPins::DRV2_NFAULT},
            {"IPROPI", (types::u8)pinmap::AnalogPins::DRV2_IPROPI},
            {"IN2", (types::u8)pinmap::DigitalPins::DRV2_IN2},
            {"IN1", (types::u8)pinmap::DigitalPins::DRV2_IN1},
            {"DRVOFF", (types::u8)pinmap::DigitalPins::DRV2_OFF}};

  case 3:
    return {{"CS", (types::u8)pinmap::DigitalPins::DRV3_SCS},
            {"MOSI", (types::u8)pinmap::DigitalPins::SPI0_MOSI},
            {"MISO", (types::u8)pinmap::DigitalPins::SPI0_MISO},
            {"SCK", (types::u8)pinmap::DigitalPins::SPI0_SCLK},
            {"NSLEEP", (types::u8)pinmap::DigitalPins::DRV3_NSLEEP},
            {"NFAULT", (types::u8)pinmap::DigitalPins::DRV3_NFAULT},
            {"IPROPI", (types::u8)pinmap::AnalogPins::DRV3_IPROPI},
            {"IN2", (types::u8)pinmap::DigitalPins::DRV3_IN2},
            {"IN1", (types::u8)pinmap::DigitalPins::DRV3_IN1},
            {"DRVOFF", (types::u8)pinmap::DigitalPins::DRV3_OFF}};

  case 4:
    return {{"CS", (types::u8)pinmap::DigitalPins::DRV4_SCS},
            {"MOSI", (types::u8)pinmap::DigitalPins::SPI0_MOSI},
            {"MISO", (types::u8)pinmap::DigitalPins::SPI0_MISO},
            {"SCK", (types::u8)pinmap::DigitalPins::SPI0_SCLK},
            {"NSLEEP", (types::u8)pinmap::DigitalPins::DRV4_NSLEEP},
            {"NFAULT", (types::u8)pinmap::DigitalPins::DRV4_NFAULT},
            {"IPROPI", (types::u8)pinmap::AnalogPins::DRV4_IPROPI},
            {"IN2", (types::u8)pinmap::DigitalPins::DRV4_IN2},
            {"IN1", (types::u8)pinmap::DigitalPins::DRV4_IN1},
            {"DRVOFF", (types::u8)pinmap::DigitalPins::DRV4_OFF}};
  default:
    return {};
  }
}