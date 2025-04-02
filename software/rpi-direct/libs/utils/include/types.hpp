#pragma once

extern "C" {
#include <stdint.h>
}

namespace types {

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

struct Vec3i16 {
    Vec3i16(i16 x, i16 y, i16 z) : x(x), y(y), z(z) {}
    Vec3i16(i16 i16_arr[3]) : x(i16_arr[0]), y(i16_arr[1]), z(i16_arr[2]) {}
    i16 x;
    i16 y;
    i16 z;
};

struct Vec3f32 {
    Vec3f32(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
    Vec3f32(Vec3i16 i16_vec) : x(i16_vec.x), y(i16_vec.y), z(i16_vec.z) {}
    f32 x, y, z;
};

} // namespace types
