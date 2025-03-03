#pragma once
#include "types.hpp"

namespace comms {

enum class CommsErrors : types::u8 {
  PACKET_RECV_OVER_MAX_BUFSIZE,
  PACKET_SEND_TOO_LONG,
  PACKET_RECV_OVER_COMMAND_LISTENER_MAXSIZE,
  ATTACH_LISTENER_NULLPTR,
  CALLING_UNATTACHED_LISTENER,
  LISTENER_NO_BUFFER,
  LISTENER_NO_BUFFER_MUTEX,
};

enum class CommsWarnings : types::u8 {
  OVERRIDE_COMMAND_LISTENER,
  LISTENER_BUFFER_MUTEX_HELD,
};

} // namespace comms
