#pragma once

#include "Vector.h"

class Camera
{
public:
    void move(const Vec3& dist);

    Vec3 getForwardVector() const;
    Vec3 getRightVector() const;

    void pitch(float deg);
    float pitch();
    void yaw(float deg);
    float yaw();
    void roll(float deg);
    float roll();

    Vec3 position;
    Vec3 rotation;
};

