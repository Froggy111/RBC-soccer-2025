#pragma once

#include "pinmap.hpp"
#include "types.hpp"
extern "C" {
#include <pico/stdlib.h>
}

//pinmap for PMW3360
typedef enum MouseSensorPinMap {
    CS,
    MOSI, 
    MISO,
    SCLK,
    MOT,
    RST
} MouseSensorPinMap;

constexpr types::u8 mouse_sensor1_pins[] = {
    static_cast<types::u8>(1),  // CS
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MOSI),   // MOSI
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MISO),   // MISO
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_SCLK),   // SCK
    static_cast<types::u8>(pinmap::DigitalPins::MOUSE1_MOT),  // MOT
    static_cast<types::u8>(pinmap::Mux1A::MOUSE1_RST),  // RST
};

constexpr types::u8 mouse_sensor2_pins[] = {
    static_cast<types::u8>(pinmap::Mux2A::MOUSE2_SCS),  // CS
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MOSI),   // MOSI
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_MISO),   // MISO
    static_cast<types::u8>(pinmap::DigitalPins::SPI0_SCLK),   // SCK
    static_cast<types::u8>(pinmap::DigitalPins::MOUSE2_MOT),  // MOT
    static_cast<types::u8>(pinmap::Mux2A::MOUSE2_RST),  // RST
};

class PinSelector {
public:
    /**
    * @brief Get the pin based on the mouse sensor ID and debug mode, using the maps above
    * 
    * @param pin 
    * @return types::u8 
    */
    types::u8 get_pin(MouseSensorPinMap pin);

    /**
    * @brief Set the mouse sensor ID
    * 
    * @param id 
    */
    void set_mouse_sensor_id(types::u8 id);

private:
    types::u8 mouseSensorId;
};