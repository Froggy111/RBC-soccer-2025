#include "libs/utils/types.hpp"

namespace comms {

enum class Errors : types::u8 {
  PARITY_FAILED = 0,
  PACKET_RECV_TOO_LONG = 1,
  PACKET_SEND_TOO_LONG = 2
};

}
