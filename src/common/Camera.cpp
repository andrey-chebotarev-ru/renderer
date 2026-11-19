#include "Camera.h"

#include <cmath>
#include <algorithm>

#include "Math.h"

Vec3 Camera::getForwardVector() const
{
    return Vec3::normalize(Vec3(cosf(rotation.x) * sinf(rotation.y), -sinf(rotation.x), cosf(rotation.x) * cosf(rotation.y)));
    //return Vec3::normalize(Vec3(sinf(rotation.y), 0, cosf(rotation.y)));
}

Vec3 Camera::getRightVector() const
{
    return Vec3::normalize(Vec3(cosf(rotation.y), 0, -sinf(rotation.y)));
}

Vec3 Camera::getUpVector() const
{
    return Vec3::normalize(Vec3::cross(getRightVector(), getForwardVector()));
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

PlaneList Camera::getFrustrum() const
{
    PlaneList frustrum;

    float fovSin = sinf(Math::degToRad(fov * 0.5f));
    float fovCos = cosf(Math::degToRad(fov * 0.5f));

    Vec3 forward = getForwardVector();
    Vec3 right = getRightVector();
    Vec3 up = getUpVector();

    Vec3 leftNormal = Vec3::normalize(forward * fovSin + right * fovCos);
    Vec3 rightNormal = Vec3::normalize(forward * fovSin - right * fovCos);
    Vec3 topNormal = Vec3::normalize(forward * fovSin - up * fovCos);
    Vec3 bottomNormal = Vec3::normalize(forward * fovSin + up * fovCos);
    Vec3 nearNormal = forward;
    Vec3 farNormal = -forward;

    Vec3 nearPoint = position + forward * nearPlaneDistance;
    Vec3 farPoint = position + forward * farPlaneDistance;

    frustrum.push_back(Plane(leftNormal, Vec3::dot(leftNormal, position)));
    frustrum.push_back(Plane(rightNormal, Vec3::dot(rightNormal, position)));
    frustrum.push_back(Plane(topNormal, Vec3::dot(topNormal, position)));
    frustrum.push_back(Plane(bottomNormal, Vec3::dot(bottomNormal, position)));
    frustrum.push_back(Plane(nearNormal, Vec3::dot(nearNormal, nearPoint)));
    frustrum.push_back(Plane(farNormal, Vec3::dot(farNormal, farPoint)));

    return frustrum;
}