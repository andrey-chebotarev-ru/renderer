#pragma once

#include <vector>

struct Vec2
{
    float x, y;

    Vec2(float x, float y) : x(x), y(y) {}

    Vec2() : x(0), y(0) {}

    Vec2 operator - () const {
        return Vec2(-x, -y);
    }

    Vec2 operator + (const Vec2& vec) const {
        return Vec2(x + vec.x, y + vec.y);
    }

    Vec2 operator - (const Vec2& vec) const {
        return Vec2(x - vec.x, y - vec.y);
    }

    Vec2 operator * (float val) const {
        return Vec2(x * val, y * val);
    }

    friend Vec2 operator * (float val, const Vec2& vec) {
        return Vec2(vec.x * val, vec.y * val);
    }

    Vec2 operator / (float val) const {
        float invVal = 1.0f / val;
        return Vec2(x * invVal, y * invVal);
    }

    friend Vec2 operator / (float val, const Vec2& vec) {
        float invVal = 1.0f / val;
        return Vec2(vec.x * invVal, vec.y * invVal);
    }

    Vec2& operator += (const Vec2& vec) {
        x += vec.x;
        y += vec.y;
        return *this;
    }

    Vec2& operator += (float val) {
        x += val;
        y += val;
        return *this;
    }

    Vec2& operator -= (const Vec2& vec) {
        x -= vec.x;
        y -= vec.y;
        return *this;
    }

    Vec2& operator -= (float val) {
        x -= val;
        y -= val;
        return *this;
    }

    Vec2& operator *= (const Vec2& vec) {
        x *= vec.x;
        y *= vec.y;
        return *this;
    }

    Vec2& operator *= (float val) {
        x *= val;
        y *= val;
        return *this;
    }

    Vec2& operator /= (const Vec2& vec) {
        x /= vec.x;
        y /= vec.y;
        return *this;
    }

    Vec2& operator /= (float val) {
        x /= val;
        y /= val;
        return *this;
    }

    float length() const {
        return sqrtf(x * x + y * y);
    }

    static Vec2 lerp(const Vec2& v1, const Vec2& v2, float t) {
        return v1 + t * (v2 - v1);
    }

    static Vec2 normalize(const Vec2& v);
    static float cross(const Vec2& vec1, const Vec2& vec2);
    static float dot(const Vec2& vec1, const Vec2& vec2);
};

struct Vec3
{
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator - () const {
        return Vec3(-x, -y, -z);
    }

    Vec3 operator + (const Vec3& vec) const {
        return Vec3(x + vec.x, y + vec.y, z + vec.z);
    }

    Vec3 operator - (const Vec3& vec) const {
        return Vec3(x - vec.x, y - vec.y, z - vec.z);
    }

    Vec3 operator * (float val) const {
        return Vec3(x * val, y * val, z * val);
    }

    friend Vec3 operator * (float val, const Vec3& vec) {
        return Vec3(vec.x * val, vec.y * val, vec.z * val);
    }

    Vec3 operator / (float val) const {
        float invVal = 1.0f / val;
        return Vec3(x * invVal, y * invVal, z * invVal);
    }

    friend Vec3 operator / (float val, const Vec3& vec) {
        float invVal = 1.0f / val;
        return Vec3(vec.x * invVal, vec.y * invVal, vec.z * invVal);
    }

    Vec3& operator += (const Vec3& vec) {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        return *this;
    }

    Vec3& operator += (float val) {
        x += val;
        y += val;
        z += val;
        return *this;
    }

    Vec3& operator -= (const Vec3& vec) {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        return *this;
    }

    Vec3& operator -= (float val) {
        x -= val;
        y -= val;
        z -= val;
        return *this;
    }

    Vec3& operator *= (const Vec3& vec) {
        x *= vec.x;
        y *= vec.y;
        z *= vec.z;
        return *this;
    }

    Vec3& operator *= (float val) {
        x *= val;
        y *= val;
        z *= val;
        return *this;
    }

    Vec3& operator /= (const Vec3& vec) {
        x /= vec.x;
        y /= vec.y;
        z /= vec.z;
        return *this;
    }

    Vec3& operator /= (float val) {
        x /= val;
        y /= val;
        z /= val;
        return *this;
    }

    bool operator == (const Vec3& vec) const {
        return x == vec.x && y == vec.y && z == vec.z;
    }

    bool operator != (const Vec3& vec) const {
        return x != vec.x || y != vec.y || z != vec.z;
    }

    float length() const {
        return sqrtf(x * x + y * y + z * z);
    }

    static Vec3 lerp(const Vec3& v1, const Vec3& v2, float t) {
        return v1 + t * (v2 - v1);
    }

    static Vec3 normalize(const Vec3& v);
    static Vec3 cross(const Vec3& v1, const Vec3& v2);
    static float dot(const Vec3& v1, const Vec3& v2);
};

using Vec2List = std::vector<Vec2>;
using Vec3List = std::vector<Vec3>;
