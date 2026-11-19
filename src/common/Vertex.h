#pragma once

#include <vector>

#include "Vector.h"

struct Vertex2D
{
    Vec2 pos;

    float uz;
    float vz;
    float iz;

    Vertex2D() {}
    Vertex2D(const Vec2& pos, float uz, float vz, float iz)
        : pos(pos), uz(uz), vz(vz), iz(iz) {
    }

    static Vertex2D lerp(const Vertex2D& v1, const Vertex2D& v2, float t);
};

struct Vertex3D
{
    Vec3 pos;
    Vec2 uv;

    Vertex3D() {}
    Vertex3D(const Vec3& pos, const Vec2& uv)
        : pos(pos), uv(uv) {
    }

    static Vertex3D lerp(const Vertex3D& v1, const Vertex3D& v2, float t);
};

using Vertex2DList = std::vector<Vertex2D>;
using Vertex3DList = std::vector<Vertex3D>;