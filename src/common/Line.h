#pragma once

#include "Vector.h"

struct Line
{
    Vec2 normal;
    float distance;

    Line() {}

    Line(const Vec2& normal, float distance)
        : normal(normal), distance(distance) {
    }

    Line flip() const {
        return Line(-normal, -distance);
    }
};