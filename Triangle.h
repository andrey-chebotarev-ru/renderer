#pragma once

#include <vector>

#include "Vertex.h"

struct Triangle2
{
    union {
        struct {
            Vertex2D v0, v1, v2;
        };
        Vertex2D v[3];
    };

    Triangle2() {}
    Triangle2(Vertex2D v0, Vertex2D v1, Vertex2D v2) : v0(v0), v1(v1), v2(v2) {}
};

struct Triangle3
{
    union {
        struct {
            Vertex3D v0, v1, v2;
        };
        Vertex3D v[3];
    };
    
    Vec3 getNormal();

    size_t texId;

    Triangle3(Vertex3D v0, Vertex3D v1, Vertex3D v2, size_t texId) : v0(v0), v1(v1), v2(v2), texId(texId) {}
    Triangle3() {}
};

typedef std::vector<Triangle2> Triangle2DList;
typedef std::vector<Triangle3> Triangle3DList;

using Polygon2D = std::vector<Vertex2D>;
using Polygon3D = std::vector<Vertex3D>;