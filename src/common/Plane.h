#pragma once

#include <vector>

#include "Vector.h"

struct Plane
{
    Vec3 normal;
    float distance = 0;

    Plane(const Vec3& normal, float distance)
        : normal(normal), distance(distance) {
    }

    Plane() {}

    bool isAxial() const
    {
        const float epsilon = 0.0001f;
        int zeroesCount = 0;
        if (fabs(normal.x) < epsilon) {
            zeroesCount++;
        }
        if (fabs(normal.y) < epsilon) {
            zeroesCount++;
        }
        if (fabs(normal.z) < epsilon) {
            zeroesCount++;
        }

        return zeroesCount == 2;
    }

    bool operator == (const Plane& plane) const {
        return normal == plane.normal && distance == plane.distance;
    }

    bool operator != (const Plane& plane) const {
        return normal != plane.normal || distance != plane.distance;
    }

    Plane flip() const {
        return Plane(-normal, -distance);
    }
};

using PlaneList = std::vector<Plane>;