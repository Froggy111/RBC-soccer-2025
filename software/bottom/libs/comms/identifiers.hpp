#pragma once
#include "libs/utils/types.hpp"

namespace comms {

enum class SendIdentifiers : types::u8 { COMMS_ERROR = 254, FATAL = 255 };

enum class RecvIdentifiers : types::u8 {

};

} // namespace comms
