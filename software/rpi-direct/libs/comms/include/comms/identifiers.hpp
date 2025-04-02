#pragma once
#include "types.hpp"

namespace comms {

// ^ SYNC WITH THE OTHER IDENTIFIER FILES ^
enum class RecvBottomPicoIdentifiers : types::u8 {
    COMMS_WARN       = 0, // warnings should be sent here
    COMMS_ERROR      = 1, // hard errors sent here
    COMMS_DEBUG      = 2, // everything should fall under here by default
    SPI_INIT_FAIL    = 3,
    LINE_SENSOR_DATA = 4,
    PING             = 254,
    BOARD_ID         = 255,
};

enum class RecvMiddlePicoIdentifiers : types::u8 {
    COMMS_WARN  = 0, // warnings should be sent here
    COMMS_ERROR = 1, // hard errors sent here
    COMMS_DEBUG = 2, // everything should fall under here by default
    IR_DATA     = 3,
    PING        = 254,
    BOARD_ID    = 255,
};

enum class RecvTopPicoIdentifiers : types::u8 {
    COMMS_WARN        = 0, // warnings should be sent here
    COMMS_ERROR       = 1, // hard errors sent here
    COMMS_DEBUG       = 2, // everything should fall under here by default
    SPI_FAIL          = 3,
    LED_LISTENER_FAIL = 4,
    IMU               = 5,
    PING              = 254,
    BOARD_ID          = 255,
};

enum class SendBottomPicoIdentifiers : types::u8 {
    MOTOR_DRIVER_CMD = 0,
    KICKER_CMD       = 1,
    PING             = 253,
    BOARD_ID         = 254,
    DEBUG_TEST_BLINK = 255,
};

enum class SendMiddlePicoIdentifiers : types::u8 {
    LEDs             = 0,
    PING             = 253,
    BOARD_ID         = 254,
    DEBUG_TEST_BLINK = 255,
};

enum class SendTopPicoIdentifiers : types::u8 {
    LEDs             = 0,
    PING             = 253,
    BOARD_ID         = 254,
    DEBUG_TEST_BLINK = 255,
};

enum class BoardIdentifiers : types::u8 {
    BOTTOM_PICO = 0,
    MIDDLE_PICO = 1,
    TOP_PICO    = 2,
    UNKNOWN     = 3
};

static const types::u16 identifier_arr_len = 256;

} // namespace comms
