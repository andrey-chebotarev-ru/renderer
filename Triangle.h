#pragma once

#include <vector>
#include <string>

#include "Vertex.h"
#include "Color.h"

struct Plane
{
    Vec3 normal;
    float distance;

    Plane(const Vec3& normal, float distance)
        : normal(normal), distance(distance) {
    }

    Plane() {}

    Plane flip() const {
        return Plane(-normal, -distance);
    }
};

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

using PlaneList = std::vector<Plane>;

using Vertex3DList = std::vector<Vertex3D>;
using Vertex2DList = std::vector<Vertex2D>;

struct Edge2D
{
    Vertex2D vert1, vert2;
    Line line;

    Edge2D(const Vertex2D& vert1, const Vertex2D& vert2);
};

struct Edge3D
{
    Vertex3D vert1, vert2;
    
    Edge3D(const Vertex3D& vert1, const Vertex3D& vert2);
};

struct Poly2D
{
    
    Vertex2DList verts;
    Color fill;

    Poly2D() {}

    Poly2D(const Vertex2DList& verts)
        : verts(verts) {}

    Poly2D(const Triangle2& tri)
    {
        verts.reserve(3);

        verts.push_back(tri.v0);
        verts.push_back(tri.v1);
        verts.push_back(tri.v2);

        fill = tri.fill;
    }

    bool isValid() const {
        return verts.size() >= 3;
    }

    size_t getSize() const {
        return verts.size();
    }

    Edge2D getEdge(int idx) const;

    float getSignedArea() const;
    float getArea() const;


    Triangle2DList triangulate() const;
    
};

struct Poly3D
{
    Vertex3DList verts;
    size_t texId;
    Color fill;

    Poly3D() {}

    Poly3D(const Vertex3DList& verts, size_t texId)
        : verts(verts), texId(texId) {}

    Poly3D(const Triangle3& tri)
    {
        verts.reserve(3);

        verts.push_back(tri.v0);
        verts.push_back(tri.v1);
        verts.push_back(tri.v2);

        texId = tri.texId;
        fill = tri.fill;
    }

    bool isValid() const {
        return verts.size() >= 3;
    }

    size_t getSize() const {
        return verts.size();
    }

    Edge3D getEdge(int idx) const;

    float getSignedArea() const;
    float getArea() const;

    Vec3 getCenter() const;
    
    Plane getPlane() const;
    Triangle3DList triangulate() const;

    static Poly3D offset(const Poly3D& poly, float distance);
};

using Poly3DList = std::vector<Poly3D>;
using Poly2DList = std::vector<Poly2D>;

struct Label
{
    Vec3 pos;
    Color color;
    const std::string label;

    Label(const Vec3& pos, const Color& color, const std::string& label)
        : pos(pos), color(color), label(label) {
    }
};

using LabelList = std::vector<Label>;