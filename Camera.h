#pragma once

#include "Vector.h"

class Camera
{
public:
    void moveForward(float distance);
    void moveRight(float distance);

    void pitch(float deg);
    float pitch();
    void yaw(float deg);
    float yaw();
    void roll(float deg);
    float roll();

    Vec3 position;
    Vec3 rotation;
};

