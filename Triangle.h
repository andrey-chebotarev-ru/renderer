#pragma once

#include <vector>

#include "Vertex.h"
#include "Color.h"

struct Triangle2
{
    union {
        struct {
            Vertex2D v0, v1, v2;
        };
        Vertex2D v[3];
    };

    Color fill;

    Triangle2() {}
    Triangle2(Vertex2D v0, Vertex2D v1, Vertex2D v2) : v0(v0), v1(v1), v2(v2) {}
};

struct Plane
{
    Vec3 normal;
    float distance;

    Plane(const Vec3& normal, float distance)
        : normal(normal), distance(distance)
    {
    }

    Plane() {}
};

struct Triangle3
{
    union {
        struct {
            Vertex3D v0, v1, v2;
        };
        Vertex3D v[3];
    };
    
    Plane getPlane() const;


    size_t texId;
    Color fill;

    Triangle3(Vertex3D v0, Vertex3D v1, Vertex3D v2, size_t texId) : v0(v0), v1(v1), v2(v2), texId(texId) {}
    Triangle3() {}
};

using Triangle2DList = std::vector<Triangle2>;
using Triangle3DList = std::vector<Triangle3>;

using Polygon2D = std::vector<Vertex2D>;
using Polygon3D = std::vector<Vertex3D>;