#include "Vector.h"

#include <cmath>

Vec3 Vec3::normalize(const Vec3& v)
{
    float invLength = 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

    return Vec3(v.x * invLength, v.y * invLength, v.z * invLength);
}

Vec3 Vec3::cross(const Vec3& vec1, const Vec3& vec2)
{
    return Vec3(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
}

float Vec3::dot(const Vec3& vec1, const Vec3& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}