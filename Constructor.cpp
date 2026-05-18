#include "Constructor.h"

#include "Vector.h"
#include "Model.h"
#include "Math.h"


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

Triangle3DList Constructor::wall(const Vec2& vec1, const Vec2& vec2, float height, size_t texId)
{
    float h = height / 2;

    Triangle3DList tris;

    tris.reserve(2);

    tris.push_back(Triangle3({ {{vec1.x, -h, vec1.y}, {0, 0}}, {{ vec2.x, -h, vec2.y}, {1, 0}}, {{vec1.x, h, vec1.y}, {0, 1}}, texId }));
    tris.push_back(Triangle3({ {{vec2.x, -h, vec2.y}, {1, 0}}, {{ vec2.x, h, vec2.y}, {1, 1}}, {{vec1.x, h, vec1.y}, {0, 1}}, texId }));
    
    return tris;
}

Triangle3DList Constructor::map(const Vec2List list, float height, size_t texId)
{
    Triangle3DList tris;

    if (list.size() < 2) {
        return tris;
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

        Triangle3DList wallTris = wall(previous, current, height, 1);

        tris.insert(tris.begin(), wallTris.begin(), wallTris.end());

        previous = current;
    }

    float h = height / 2;

    Vec2 floor = Vec2((max.x - min.x) * 10 / 512, (max.y - min.y) * 10 / 512);

    // Floor
    tris.push_back(Triangle3({ {{min.x, h, min.y}, {0, 0}}, {{ max.x, h, max.y}, {floor.x, floor.y}}, {{min.x, h, max.y}, {0, floor.y}}, 3 }));
    tris.push_back(Triangle3({ {{min.x, h, min.y}, {0, 0}}, {{ max.x, h, min.y}, {floor.x, 0}}, {{max.x, h, max.y}, {floor.x, floor.y}}, 3 }));

    // Ceil
    tris.push_back(Triangle3({ {{min.x, -h, max.y}, {0, floor.y}}, {{ max.x, -h, max.y}, {floor.x, floor.y}}, {{min.x, -h, min.y}, {0, 0}}, 4 }));
    tris.push_back(Triangle3({ {{max.x, -h, max.y}, {floor.x, floor.y}}, {{ max.x, -h, min.y}, {floor.x, 0}}, {{min.x, -h, min.y}, {0, 0}}, 4 }));

    return tris;

}

