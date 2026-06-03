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

    bool isAxial()
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

    int texId;
    Color fill;

    Triangle3(Vertex3D v0, Vertex3D v1, Vertex3D v2, int texId) : v0(v0), v1(v1), v2(v2), texId(texId) {}
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

        addVert(tri.v0);
        addVert(tri.v1);
        addVert(tri.v2);

        fill = tri.fill;
    }

    bool isValid() const {
        return verts.size() >= 3;
    }

    size_t getCount() const {
        return verts.size();
    }

    int getEdgeCount() const {
        return verts.size();
    }

    void addVert(const Vertex2D& vert) {
        verts.push_back(vert);
    }

    Edge2D getEdge(int idx) const;

    float getSignedArea() const;
    float getArea() const;

    Triangle2DList triangulate() const;
    
};

struct Poly3D
{
    Vertex3DList verts;
    

    void copyPropertiesFrom(const Poly3D& poly)
    {


    }

    Poly3D() {}

    Poly3D(const Vertex3DList& verts)
        : verts(verts) {}

    Poly3D(const Triangle3& tri)
    {
        verts.reserve(3);

        addVert(tri.v0);
        addVert(tri.v1);
        addVert(tri.v2);
    }

    bool isValid() const {
        return verts.size() >= 3;
    }

    size_t getCount() const {
        return verts.size();
    }

    int getEdgeCount() const {
        return verts.size();
    }

    void addVert(const Vertex3D& vert) {
        verts.push_back(vert);
    }

    void clear() {
        verts.clear();
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

enum class FaceType
{
    Solid,
    Bounds,
    Debug,
};

struct Face
{
    FaceType type = FaceType::Solid;

    Poly3D poly;

    int texIdx = 0;
    Color debugColor;

    Face() {};
    Face(const Poly3D& poly, int texIdx, FaceType type = FaceType::Solid, Color debugColor = Color(255, 255, 255))
        : type(type), poly(poly), texIdx(texIdx), debugColor(debugColor) {
    }

    Face(const Face& face, Poly3D& poly)
        : poly(poly)
    {
        copyPropsFrom(face);
    }

    void copyPropsFrom(const Face& face)
    {
        texIdx = face.texIdx;
        debugColor = face.debugColor;
        type = face.type;
    }
};

using FaceList = std::vector<Face>;

struct AABB2D
{
    Vec2 min, max;
};

struct AABB3D
{
    Vec3 min, max;
};