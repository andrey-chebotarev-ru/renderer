#include "Constructor.h"

#include "Vector.h"
#include "Model.h"

Model Constructor::room(const Vec3& position, const Vec3& rotation, const Vec2& uv, const Vec3& size, size_t textureIndex)
{
	Vec3 sideSize = size / 2;

	Model model = Model(position, rotation, textureIndex, {});

    model.addTriangle(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {uv.x, uv.y}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, uv.y}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {uv.x, uv.y}}, {{-sideSize.x, sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, uv.y}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, uv.y}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {uv.x, 0}}, {{sideSize.x, -sideSize.y, sideSize.z}, {uv.x, uv.y}} }));

    return model;

}

Model Constructor::cube(const Vec3& position, const Vec3& rotation, const Vec3& size, size_t textureIndex)
{

    Vec3 sideSize = size / 2;

    Model model = Model(position, rotation, textureIndex, {});

    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {1, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {1, 1}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}} }));
    model.addTriangle(Triangle3({ {{ -sideSize.x, -sideSize.y, sideSize.z }, {0, 0}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {0, 1}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, sideSize.z}, {0, 1}} , {{sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} , {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}} }));
    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 1}} }));

    model.addTriangle(Triangle3({ {{-sideSize.x, sideSize.y, -sideSize.z}, {0, 0}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, sideSize.y, sideSize.z}, {1, 1}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{-sideSize.x, -sideSize.y, -sideSize.z}, {0, 0}} }));
    model.addTriangle(Triangle3({ {{sideSize.x, -sideSize.y, sideSize.z}, {1, 1}}, {{-sideSize.x, -sideSize.y, sideSize.z}, {1, 0}}, {{sideSize.x, -sideSize.y, -sideSize.z}, {0, 1}} }));

    return model;
}

Model Constructor::test(const Vec3& position, const Vec3& rotation, size_t textureIndex)
{
    Model model = Model(position, rotation, textureIndex, {});

    //model.addTriangle(Triangle3({ {{-100, 100, 0}, {0, 1}}, {{100, -100, 0}, {1, 0}}, {{-100, -100, 0}, {0, 0}}, textureIndex }));
    model.addTriangle(Triangle3({ {{100, 100, 0}, {1, 1}}, {{100, -100, 0}, {1, 0}}, {{-100, 100, 0}, {0, 1}} }));

    return model;
}