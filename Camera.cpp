#include "Camera.h"

#include <cmath>
#include <algorithm>

#include "Math.h"

Vec3 Camera::getForwardVector()
{
    return Vec3::normalize(Vec3(cosf(rotation.x) * sinf(rotation.y), -sinf(rotation.x), cosf(rotation.x) * cosf(rotation.y)));
    //return Vec3::normalize(Vec3(sinf(rotation.y), 0, cosf(rotation.y)));
}

Vec3 Camera::getRightVector()
{
    return Vec3::normalize(Vec3(cosf(rotation.y), 0, -sinf(rotation.y)));
}

void Camera::move(const Vec3& dist)
{
    position += dist;
}

void Camera::pitch(float deg)
{
    float min = Math::degToRad(-90);
    float max = Math::degToRad(90);

    rotation.x += Math::degToRad(deg);
    rotation.x = std::clamp(rotation.x, min, max);
}

void Camera::yaw(float deg)
{
    rotation.y += Math::degToRad(deg);
}

void Camera::roll(float deg)
{
    rotation.z += Math::degToRad(deg);
}