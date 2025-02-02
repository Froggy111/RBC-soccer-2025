#pragma once

#include <stdint.h>
// #include <stdfloat>

namespace types {
  typedef uint8_t u8;
  typedef uint16_t u16;
  typedef uint32_t u32;
  typedef uint64_t u64;
  typedef __uint128_t u128; // do not use unless absolutely necessary!

  typedef int8_t i8;
  typedef int16_t i16;
  typedef int32_t i32;
  typedef int64_t i64;
  typedef __int128_t i128; // do not use unless absolutely necessary!

  // typedef std::float16_t f16;
  // typedef std::float32_t f32;
  // typedef std::float64_t f64;
  // typedef std::float128_t f128;
  // typedef std::bfloat16_t bf16;

  typedef float f64;
  typedef unsigned char byte;
}