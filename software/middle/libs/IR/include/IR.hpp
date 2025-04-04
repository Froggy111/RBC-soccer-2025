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
  types::u64 last_fall = 0;
  types::u32 uptime = 0;
  inline void reset(void) volatile { last_fall = time_us_64(), uptime = 0; }
  inline void zero(void) volatile { uptime = 0; }
};

struct ModulationData {
  types::u32 uptime = 0;
  inline void reset(void) volatile { uptime = 0; }
};

void init(void);

// modulation timer
const uint MODULATION_IRQ = TIMER_IRQ_0;
const double MODULATION_FREQ = 1200 / 16; // 833.333us per cycle
const types::u32 CYCLES_PER_MODULATION = (double)SYS_CLK_HZ / MODULATION_FREQ;
const types::u8 MODULATION_ALARM_IDX = 0;
static volatile ModulationData modulation_data[SENSOR_COUNT];
// calculate, and pass task notification to modulation_handler_task to send data
static void modulation_handler(void);
// send data to host
static void modulation_handler_task(void *params);
static TaskHandle_t modulation_handler_task_handle;
const types::u16 MODULATION_HANLDER_TASK_STACK_SIZE = 1024;
const types::u16 MODULATION_HANLDER_TASK_PRIORITY = 12;

// // pulse timer
const double PULSE_FREQ = 40000;
const types::u32 max_time_passed = 1e6 / PULSE_FREQ * 2;
// const types::u32 CYCLES_PER_PULSE = (double)SYS_CLK_HZ / PULSE_FREQ;
// const types::u8 PULSE_ALARM_IDX = 1;
static volatile PulseData pulse_data[SENSOR_COUNT];
static void pulse_handler(uint gpio,
                          types::u32 event); // handles GPIO interrupts
} // namespace IR
