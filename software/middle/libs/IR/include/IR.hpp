#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"
#include "pinmap.hpp"

extern "C" {
#include <hardware/timer.h>
#include <hardware/irq.h>
}

namespace IR {
// constants
const types::u8 SENSOR_COUNT = 24;
const types::u8 SENSOR_PINS[SENSOR_COUNT] = {
    (types::u8)pinmap::Pico::IR1,  (types::u8)pinmap::Pico::IR2,
    (types::u8)pinmap::Pico::IR3,  (types::u8)pinmap::Pico::IR4,
    (types::u8)pinmap::Pico::IR5,  (types::u8)pinmap::Pico::IR6,
    (types::u8)pinmap::Pico::IR7,  (types::u8)pinmap::Pico::IR8,
    (types::u8)pinmap::Pico::IR9,  (types::u8)pinmap::Pico::IR10,
    (types::u8)pinmap::Pico::IR11, (types::u8)pinmap::Pico::IR12,
    (types::u8)pinmap::Pico::IR13, (types::u8)pinmap::Pico::IR14,
    (types::u8)pinmap::Pico::IR15, (types::u8)pinmap::Pico::IR16,
    (types::u8)pinmap::Pico::IR17, (types::u8)pinmap::Pico::IR18,
    (types::u8)pinmap::Pico::IR19, (types::u8)pinmap::Pico::IR20,
    (types::u8)pinmap::Pico::IR21, (types::u8)pinmap::Pico::IR22,
    (types::u8)pinmap::Pico::IR23, (types::u8)pinmap::Pico::IR24,
};

struct PulseData {
  types::u64 last_rise;
  types::u32 uptime;
  void reset(void) { uptime = 0; }
};

// modulation timer
const double MODULATION_FREQ = 1200; // 833.333us per cycle
const types::u32 CYCLES_PER_MODULATION = (double)SYS_CLK_HZ / MODULATION_FREQ;
const types::u8 MODULATION_ALARM_IDX = 0;
static void modulation_handler(
    void); // calculate, and pass task notification to modulation_handler_task to send data
static void modulation_handler_task(void); // send data to host

// // pulse timer
// const double PULSE_FREQ = 40000;
// const types::u32 CYCLES_PER_PULSE = (double)SYS_CLK_HZ / PULSE_FREQ;
// const types::u8 PULSE_ALARM_IDX = 1;
static volatile PulseData pulse_data[SENSOR_COUNT];
static void pulse_handler(void); // handles GPIO interrupts
} // namespace IR
