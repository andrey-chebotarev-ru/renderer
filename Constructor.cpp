#include "Constructor.h"

#include "Vector.h"
#include "Model.h"
#include "Math.h"

Triangle3DList Constructor::wall(const Vec2& uv, const Vec2& size, const Transform& tr, size_t texId)
{
    Vec2 sideSize = size / 2;

    Triangle3DList tris;

    tris.reserve(2);

    tris.push_back(Triangle3({ {{-sideSize.x, sideSize.y, 0}, {0, 1}}, {{sideSize.x, -sideSize.y, 0}, {1, 0}}, {{-sideSize.x, -sideSize.y, 0}, {0, 0}}, texId }));
    tris.push_back(Triangle3({ {{sideSize.x, sideSize.y, 0}, {1, 1}}, {{sideSize.x, -sideSize.y, 0}, {1, 0}}, {{-sideSize.x, sideSize.y, 0}, {0, 1}}, texId }));

    for (Triangle3& tri : tris) {
        for (int i = 0; i < 3; i++) {
            tri.v[i].position = Math::transform(tri.v[i].position, tr);
        }
    }

    return tris;
}

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