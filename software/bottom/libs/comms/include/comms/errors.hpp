#pragma once
#include "types.hpp"

namespace comms {

enum class CommsErrors : types::u8 {
  PARITY_FAILED = 0,
  PACKET_RECV_TOO_LONG = 1,
  PACKET_SEND_TOO_LONG = 2,
  RECIEVED_MORE_THAN_EXPECTED = 3,
};

}
