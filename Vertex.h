#pragma once

#include<vector>

#include "Vector.h"

struct Vertex2D
{
    Vec2 pos;

    float uz;
    float vz;
    float iz;

    static Vertex2D lerp(const Vertex2D& v1, const Vertex2D& v2, float t);
};

struct Vertex3D
{
    Vec3 position;
    Vec2 uv;

    Vertex3D(Vec3 position, Vec2 uv) : position(position), uv(uv) {}
    Vertex3D() {}

    static Vertex3D lerp(const Vertex3D& v1, const Vertex3D& v2, float t);
};

typedef std::vector<Vertex2D> Vertex2DList;
typedef std::vector<Vertex3D> Vertex3DList;
