#include "Math.h"

const float Math::pi = 3.1415f;

Vec2 Math::rotate(const Vec2& vec, float roll)
{
    Vec2 out;

    float c = cosf(roll);
    float s = sinf(roll);

    out.x = vec.x * c - vec.y * s;
    out.y = vec.x * s + vec.y * c;
    
    return out;
}

Vec3 Math::rotateY(const Vec3& vec, float yaw)
{
    Vec3 out;

    float c = cosf(yaw);
    float s = sinf(yaw);

    out.x = vec.x * c + vec.z * s;
    out.y = vec.y;
    out.z = vec.z * c - vec.x * s;

    return out;
}

Vec3 Math::rotateX(const Vec3& vec, float pitch)
{
    Vec3 out;

    float c = cosf(pitch);
    float s = sinf(pitch);

    out.x = vec.x;
    out.y = vec.y * c - vec.z * s;
    out.z = vec.y * s + vec.z * c;

    return out;
}

Vec3 Math::rotateZ(const Vec3& vec, float roll)
{
    Vec3 out;

    float c = cosf(roll);
    float s = sinf(roll);

    out.x = vec.x * c - vec.y * s;
    out.y = vec.x * s + vec.y * c;
    out.z = vec.z;

    return out;
}

Vec3 Math::rotate(const Vec3& vec, const Vec3& rot)
{
    Vec3 out = rotateY(vec, rot.y);
    
    out = rotateX(out, rot.x);
    
    return rotateZ(out, rot.z);
}

Vec3 Math::transform(const Vec3& vec, const Transform& tr)
{
    Vec3 out;

    // Scale
    out.x = vec.x * tr.scale.x;
    out.y = vec.y * tr.scale.y;
    out.z = vec.z * tr.scale.z;

    // Rotate
    out = rotate(out, tr.rotation);
    
    // Translate
    out += tr.position;

    return out;
}

float Math::distanceToPlane(const Plane& plane, const Vec3& vec)
{
    return Vec3::dot(plane.normal, vec) - plane.distance;
}

float Math::distanceToLine(const Line& line, const Vec2& vec)
{
    return Vec2::dot(line.normal, vec) - line.distance;
}