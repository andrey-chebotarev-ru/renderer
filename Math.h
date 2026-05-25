#pragma once

#include <cstdint>

#include "Vector.h"
#include "Triangle.h"

typedef int32_t fixed_t;

struct Transform
{
    Vec3 scale;
    Vec3 rotation;
    Vec3 position;
    
    Transform(Vec3 scale, Vec3 rotation, Vec3 position)
        : scale(scale), rotation(rotation), position(position)
    {
    }
};


class Math
{
public:
    static const float pi;
    static const int fpBits = 16;

    static inline float degToRad(float deg) {
        return deg * pi / 180.0f;
    }
    static inline float radToDeg(float rad) {
        return rad * 180.0f / pi;
    }
    static inline bool isPow2(int val) {
        return (val != 0) && ((val & (val - 1)) == 0);
    }
    static int shift(int val)
    {
        int shift = 0;

        while (val > 1) {
            val >>= 1;
            shift++;
        }

        return shift;
    }

    static inline int32_t fixedToInt(fixed_t val) {
        return val >> fpBits;
    }

    static inline float fixedToFloat(fixed_t val) {
        return float(val) / float(1 << fpBits);
    }

    static inline fixed_t intToFixed(int32_t val) {
        return val << fpBits;
    }

    static inline fixed_t floatToFixed(float val) {
        return fixed_t(val * (1 << fpBits));
    }

    static inline fixed_t fixedMul(fixed_t a, fixed_t b) {
        return ((int64_t(a) * b) + 0x8000) >> fpBits;
    }

    static inline fixed_t fixedDiv(fixed_t a, fixed_t b) {
        return ((int64_t(a) << fpBits) / b);
    }

    static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    static Vec2 rotate(const Vec2& vec, float roll);
    static Vec3 rotateY(const Vec3& vec, float yaw);
    static Vec3 rotateX(const Vec3& vec, float pitch);
    static Vec3 rotateZ(const Vec3& vec, float roll);
    static Vec3 rotate(const Vec3& vec, const Vec3& rot);
    static Vec3 transform(const Vec3& vec, const Transform& tr);
    static float distanceToPlane(const Plane& plane, const Vec3& vec);
    static float distanceToLine(const Line& line, const Vec2& vec);
};