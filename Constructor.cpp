#include "Constructor.h"

Triangle3DList Constructor::room(const Vec2& uv, const Vec3& size, size_t texId)
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

Triangle3DList Constructor::cube(const Vec2& uv, const Vec3& size, size_t texId)
{
    Vec3 sideSize = size / 2;

    Triangle3DList tris;

    tris.reserve(12);

    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {1, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {1, 1}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    tris.push_back(Triangle3({ {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, 1}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {0, 1}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} , {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 1}}, texId }));

    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, -sideSize.y, sideSize.z}, {1, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 1}}, texId }));

    return tris;
}

Poly3DList Constructor::wall(const Vec2& vec1, const Vec2& vec2, float height, size_t texId)
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
        }, texId));

    return polys;
}

Poly3DList Constructor::map(const Vec2List list, float height, size_t texId)
{
    Poly3DList polys;

    if (list.size() < 2) {
        return polys;
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

        Poly3DList wallPolys = wall(previous, current, height, 1);

        polys.insert(polys.begin(), wallPolys.begin(), wallPolys.end());

        previous = current;
    }

    float h = height / 2;

    Vec2 uv = Vec2((max.x - min.x) * 10 / 512, (max.y - min.y) * 10 / 512);

    // Floor
    polys.push_back(Poly3D({ {{min.x, h, min.y}, {0, 0}}, {{ max.x, h, max.y}, {uv.x, uv.y}}, {{min.x, h, max.y}, {0, uv.y}} }, 3));
    polys.push_back(Poly3D({ {{min.x, h, min.y}, {0, 0}}, {{ max.x, h, min.y}, {uv.x, 0}}, {{max.x, h, max.y}, {uv.x, uv.y}} }, 3));

    // Ceil
    polys.push_back(Poly3D({ {{min.x, -h, max.y}, {0, uv.y}}, {{ max.x, -h, max.y}, {uv.x, uv.y}}, {{min.x, -h, min.y}, {0, 0}} }, 4));
    polys.push_back(Poly3D({ {{max.x, -h, max.y}, {uv.x, uv.y}}, {{ max.x, -h, min.y}, {uv.x, 0}}, {{min.x, -h, min.y}, {0, 0}} }, 4));

    return polys;

}

