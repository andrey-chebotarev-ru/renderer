#include "Constructor.h"

#include "Vector.h"
#include "Model.h"

Triangle3DList Constructor::room(const Vec2& uv, const Vec3& size, size_t texId)
{
	Vec3 sideSize = size / 2;

    Triangle3DList triangles;

    triangles.reserve(12);

    triangles.push_back(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {uv.x, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, uv.y}}, texId }));

    return triangles;
}

Triangle3DList Constructor::cube(const Vec2& uv, const Vec3& size, size_t texId)
{
    Vec3 sideSize = size / 2;

    Triangle3DList triangles;

    triangles.reserve(12);

    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {1, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {1, 1}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    triangles.push_back(Triangle3({ {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, 1}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {0, 1}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} , {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 1}}, texId }));

    triangles.push_back(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, texId }));
    triangles.push_back(Triangle3({ {{sideSize.x, -sideSize.y, sideSize.z}, {1, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 1}}, texId }));

    return triangles;
}