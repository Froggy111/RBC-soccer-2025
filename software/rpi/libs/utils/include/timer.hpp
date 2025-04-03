#include "types.hpp"
#include <time.h>

namespace timer {
static timespec ts;

inline types::u64 us(void) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    types::u64 time_us = ts.tv_sec * 1e6 + (float)ts.tv_nsec * 1e-3;
    return time_us;
}
} // namespace timer
