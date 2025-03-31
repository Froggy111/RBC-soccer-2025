#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"

extern "C" {
#include <hardware/timer.h>
#include <hardware/irq.h>
}

namespace IR {
const double IR_MODULATION_FREQ = 1200; // 833.333us per cycle
const types::u32 CYCLES_PER_MODULATION =
    (double)SYS_CLK_HZ / IR_MODULATION_FREQ;
const double IR_FREQ = 40000;
const types::u32 CYCLES_PER_PULSE = (double)SYS_CLK_HZ / IR_FREQ;
const types::u8 ALARM_NUM = 0;

static void timer_irq_handler(void);
} // namespace IR
