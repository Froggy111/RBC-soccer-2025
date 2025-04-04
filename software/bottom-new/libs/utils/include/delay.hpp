#pragma once

/**
 * @brief Provides precise nanosecond delay using compiler-generated noop instructions
 * @details This template recursively generates a sequence of noop instructions at compile time,
 *          allowing for fixed-cycle delays with minimal overhead
 */
namespace utils {

// Base implementation that unrolls to N consecutive noop instructions
template <unsigned N>
struct DelayNS {
    static inline void delay() {
        asm volatile("nop");
        DelayNS<N-1>::delay();
    }
};

// Specialization for terminating the recursion
template <>
struct DelayNS<0> {
    static inline void delay() {}
};

// Helper function to make the syntax cleaner
template <unsigned N>
static inline void delay_ns() {
    DelayNS<N>::delay();
}

} // namespace utils