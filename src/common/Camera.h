#pragma once

#include "Vector.h"
#include "Plane.h"

class Camera
{
public:

    Camera(float fov, float nearPlaneDistance, float farPlaneDistance)
        : fov(fov), nearPlaneDistance(nearPlaneDistance), farPlaneDistance(farPlaneDistance) {
    }

    void move(const Vec3& dist);

    Vec3 getForwardVector() const;
    Vec3 getRightVector() const;
    Vec3 getUpVector() const;
    PlaneList getFrustrum() const;

    void pitch(float deg);
    float pitch();
    void yaw(float deg);
    float yaw();
    void roll(float deg);
    float roll();

    float fov;
    float nearPlaneDistance;
    float farPlaneDistance;
    
    Vec3 position;
    Vec3 rotation;
};
