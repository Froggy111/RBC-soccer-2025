#pragma once
#include "types.hpp"

namespace comms {

enum class CommsErrors : types::u8 {
  PACKET_RECV_OVER_MAX_BUFSIZE,
  PACKET_SEND_TOO_LONG,
  PACKET_RECV_OVER_COMMAND_LISTENER_MAXSIZE,
  DEVICE_CONNECTION_FAILED,
  DEVICE_NOT_FOUND,
  USB_INITIALIZATION_FAILED,
  WRITE_FAILED
};

enum class CommsWarnings : types::u8 {
  OVERRIDE_COMMAND_LISTENER,
  DEVICE_ALREADY_CONNECTED,
  MISSING_CALLBACK
};

} // namespace comms