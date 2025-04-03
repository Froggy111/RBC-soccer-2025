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
    f32 x, y, z;

    // Addition
    Vec3f32 operator+(const Vec3f32 &other) const {
        return Vec3f32(x + other.x, y + other.y, z + other.z);
    }

    // Subtraction
    Vec3f32 operator-(const Vec3f32 &other) const {
        return Vec3f32(x - other.x, y - other.y, z - other.z);
    }

    // Scalar multiplication
    Vec3f32 operator*(f32 scalar) const {
        return Vec3f32(x * scalar, y * scalar, z * scalar);
    }

    // Scalar division
    Vec3f32 operator/(f32 scalar) const {
        return Vec3f32(x / scalar, y / scalar, z / scalar);
    }

    // multiply member by member
    Vec3f32 operator*(const Vec3f32 &other) const {
        return Vec3f32(x * other.x, y * other.y, z * other.z);
    }

    f32 dot(const Vec3f32 &other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Cross product
    Vec3f32 cross(const Vec3f32 &other) const {
        return Vec3f32(y * other.z - z * other.y, z * other.x - x * other.z,
                       x * other.y - y * other.x);
    }

    // Compound assignment operators
    Vec3f32 &operator+=(const Vec3f32 &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3f32 &operator-=(const Vec3f32 &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3f32 &operator*=(f32 scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3f32 &operator/=(f32 scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // Equality operators
    bool operator==(const Vec3f32 &other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Vec3f32 &other) const { return !(*this == other); }

    // Negation
    Vec3f32 operator-() const { return Vec3f32(-x, -y, -z); }
};

} // namespace types
