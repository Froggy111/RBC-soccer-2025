#pragma once

#include "pinmap.hpp"
#include "types.hpp"
#include <cstdint>
extern "C" {
#include <pico/stdlib.h>
#include <map>
#include <utility>
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

constexpr types::u8 driver1_pins[] = {
    static_cast<types::u8>(pinmap::Mux1B::DRV1_SCS),        // CS
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MOSI), // MOSI
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MISO), // MISO
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_SCLK), // SCK
    static_cast<types::u8>(pinmap::Mux1B::DRV1_NSLEEP),     // NSLEEP
    static_cast<types::u8>(pinmap::Mux2B::DRV1_NFAULT),     // NFAULT
    static_cast<types::u8>(pinmap::ADC2::DRV1_IPROPI),      // IPROPI
    static_cast<types::u8>(pinmap::DigitalPins::DRV1_IN2),  // IN2
    static_cast<types::u8>(pinmap::DigitalPins::DRV1_IN1),  // IN1
    static_cast<types::u8>(pinmap::Mux1B::DRV1_OFF)         // DRVOFF
};

inline bool driver1_mux_addr[][2] = {
    // on board 1, on bank A
    {true, false},  {0, 0}, {0, 0}, {0, 0}, {true, false},
    {false, false}, {0, 0}, {0, 0}, {0, 0}, {true, false}};

constexpr types::u8 driver2_pins[] = {
    static_cast<types::u8>(pinmap::Mux2B::DRV2_SCS),        // CS
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MOSI), // MOSI
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MISO), // MISO
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_SCLK), // SCK
    static_cast<types::u8>(pinmap::Mux2A::DRV2_NSLEEP),     // NSLEEP
    static_cast<types::u8>(pinmap::Mux2A::DRV2_NFAULT),     // NFAULT
    static_cast<types::u8>(pinmap::ADC1::DRV2_IPROPI),      // IPROPI
    static_cast<types::u8>(pinmap::DigitalPins::DRV2_IN2),  // IN2
    static_cast<types::u8>(pinmap::DigitalPins::DRV2_IN1),  // IN1
    static_cast<types::u8>(pinmap::Mux2B::DRV2_OFF)         // DRVOFF
};

constexpr types::u8 driver3_pins[] = {
    static_cast<types::u8>(pinmap::Mux2A::DRV3_SCS),        // CS
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MOSI), // MOSI
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MISO), // MISO
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_SCLK), // SCK
    static_cast<types::u8>(pinmap::Mux2A::DRV3_NSLEEP),     // NSLEEP
    static_cast<types::u8>(pinmap::Mux2A::DRV3_NFAULT),     // NFAULT
    static_cast<types::u8>(pinmap::ADC1::DRV3_IPROPI),      // IPROPI
    static_cast<types::u8>(pinmap::DigitalPins::DRV3_IN2),  // IN2
    static_cast<types::u8>(pinmap::DigitalPins::DRV3_IN1),  // IN1
    static_cast<types::u8>(pinmap::Mux2A::DRV3_OFF)         // DRVOFF
};

constexpr types::u8 driver4_pins[] = {
    static_cast<types::u8>(pinmap::Mux1B::DRV4_SCS),        // CS
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MOSI), // MOSI
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MISO), // MISO
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_SCLK), // SCK
    static_cast<types::u8>(pinmap::Mux1B::DRV4_NSLEEP),     // NSLEEP
    static_cast<types::u8>(pinmap::Mux2B::DRV4_NFAULT),     // NFAULT
    static_cast<types::u8>(pinmap::ADC2::DRV4_IPROPI),      // IPROPI
    static_cast<types::u8>(pinmap::DigitalPins::DRV4_IN2),  // IN2
    static_cast<types::u8>(pinmap::DigitalPins::DRV4_IN1),  // IN1
    static_cast<types::u8>(pinmap::Mux1A::DRV4_OFF)         // DRVOFF
};

constexpr types::u8 debug_pins[] = {
    1, // CS
    3, // MOSI
    0, // MISO
    2, // SCK
    9, // NSLEEP
    7, // NFAULT
    8, // IPROPI
    5, // IN2
    4, // IN1
    6  // DRVOFF
};

class PinSelector {
public:
  /**
   * @brief Get the pin based on the driver ID and debug mode, using the maps above
   * 
   * @param pin 
   * @return types::u8 
   */
  types::u8 get_pin(DriverPinMap pin);

  /**
   * @brief Get the on_A based on the driver ID and debug mode, using the maps above
   * 
   * @param pin 
   * @return pair<bool, bool> 
   */
  bool get_On_A(DriverPinMap pin);

  /**
   * @brief Get the on_board_1 based on the driver ID and debug mode, using the maps above
   * 
   * @param pin 
   * @return pair<bool, bool> 
   */
  bool get_On_Board1(DriverPinMap pin);

  /**
   * @brief Set the driver ID
   * 
   * @param id 
   */
  void set_driver_id(types::u8 id);

  /**
   * @brief Set the debug mode object
   * 
   * @param mode 
   */
  void set_debug_mode(bool mode);

private:
  types::u8 driverId;
  bool debugMode;
};