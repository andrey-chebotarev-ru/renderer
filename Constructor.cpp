#include "Constructor.h"

Triangle3DList Constructor::room(const Vec2& uv, const Vec3& size, int texId)
{
	Vec3 sideSize = size / 2;

    Triangle3DList tris;

    tris.reserve(12);

    tris.push_back(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {uv.x, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, uv.y}}, texId }));

    return tris;
}

Triangle3DList Constructor::cube(const Vec2& uv, const Vec3& size, int texId)
{
    Vec3 sideSize = size / 2;

    Triangle3DList tris;

    tris.reserve(12);

    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {uv.x, uv.y}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}} , {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}} , {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}} , {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} , {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, texId }));

    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));

    return tris;
}


Poly3DList Constructor::polyCube(const Vec3& size)
{
    Vec3 sideSize = size / 2;
    const Vec2 uv1 = Vec2(size.x * 10 / 512, size.y * 10 / 512);
    const Vec2 uv2 = Vec2(size.z * 10 / 512, size.y * 10 / 512);
    const Vec2 uv3 = Vec2(size.z * 10 / 512, size.x * 10 / 512);

    Poly3DList polys;

    polys.reserve(6);

    polys.push_back(Poly3D({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv1.y}}, {{sideSize.x, sideSize.y, -sideSize.z}, {uv1.x, uv1.y}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv1.x, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} }));
    polys.push_back(Poly3D({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv2.y}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv2.x, uv2.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv2.x, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} }));
    polys.push_back(Poly3D({ {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}} , {{sideSize.x, -sideSize.y, sideSize.z}, {uv1.x, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv1.x, uv1.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv1.y}} }));
    polys.push_back(Poly3D({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} , {{-sideSize.x, -sideSize.y, sideSize.z}, {uv2.x, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv2.x, uv2.y}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv2.y}} }));
    polys.push_back(Poly3D({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv3.x, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv3.x, uv3.y}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv3.y}} }));
    polys.push_back(Poly3D({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv3.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv3.x, uv3.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv3.x, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} }));

    return polys;
}

Poly3DList Constructor::wall(const Vec2& vec1, const Vec2& vec2, float height)
{
    float h = height / 2;

    Poly3DList polys;

    polys.reserve(1);

    float len = (vec2 - vec1).length();

    Vec2 uv = Vec2(len * 10 / 512, height * 10 / 512);

    polys.push_back(Poly3D({
        { {vec1.x, -h, vec1.y}, {0, 0} },
        { {vec2.x, -h, vec2.y}, {uv.x, 0} },
        { {vec2.x, h, vec2.y}, {uv.x, uv.y} },
        { {vec1.x, h, vec1.y}, {0, uv.y} },
        }));

    return polys;
}

FaceList Constructor::map(const Vec2List list, float height, int wallTexId, int floorTexId, int ceilTexId)
{
    FaceList faces;

    if (list.size() < 2) {
        return faces;
    }

    Vec2 previous = list.back();
    Vec2 min = previous, max = previous;

    for (const Vec2& current : list) {

        if (current.x < min.x) {
            min.x = current.x;
        }
        if (current.y < min.y) {
            min.y = current.y;
        }

        if (current.x > max.x) {
            max.x = current.x;
        }
        if (current.y > max.y) {
            max.y = current.y;
        }

        Poly3DList wallPolys = wall(previous, current, height);

        insert(faces, wallPolys, wallTexId);

        previous = current;
    }

    float h = height / 2;

    Vec2 uv = Vec2((max.x - min.x) * 10 / 512, (max.y - min.y) * 10 / 512);

    // Floor
    faces.push_back(Face(Poly3D({ {{min.x, h, min.y}, {0, 0}}, {{ max.x, h, max.y}, {uv.x, uv.y}}, {{min.x, h, max.y}, {0, uv.y}} }), floorTexId));
    faces.push_back(Face(Poly3D({ {{min.x, h, min.y}, {0, 0}}, {{ max.x, h, min.y}, {uv.x, 0}}, {{max.x, h, max.y}, {uv.x, uv.y}} }), floorTexId));

    // Ceil
    faces.push_back(Face(Poly3D({ {{min.x, -h, max.y}, {0, uv.y}}, {{ max.x, -h, max.y}, {uv.x, uv.y}}, {{min.x, -h, min.y}, {0, 0}} }), ceilTexId));
    faces.push_back(Face(Poly3D({ {{max.x, -h, max.y}, {uv.x, uv.y}}, {{ max.x, -h, min.y}, {uv.x, 0}}, {{min.x, -h, min.y}, {0, 0}} }), ceilTexId));

    return faces;

}

void Constructor::insert(FaceList& faces, const Poly3DList& polys, int texIdx, const Vec3& pos, const Vec3& rot)
{
    Transform tr = Transform(Vec3(1, 1, 1), rot, pos);

    for (auto poly : polys) {

        for (int i = 0; i < poly.getCount(); i++) {
            poly.verts[i].pos = Math::transform(poly.verts[i].pos, tr);
        }

        faces.push_back(Face(poly, texIdx));
    }
}
