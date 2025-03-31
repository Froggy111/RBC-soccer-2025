#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include "IR.hpp"

extern "C" {
#include <hardware/timer.h>
#include <hardware/irq.h>
}

using namespace types;

namespace IR {

void timer_irq_handler(void) {
  u32 current_alarm_target = timer_hw->alarm[ALARM_NUM];
}

} // namespace IR
