#include "BSP.h"

PlaneSide test(const Triangle3& tri, const Vec3& vec)
{
	const float epsilon = 0.0001f;

	float side = tri.test(vec);

	if (side > 0.0f) {
		return Front;
	}
	else {
		return Back;
	}
}

Node* BSP::compile(const Triangle3DList& tris)
{


	return nullptr;

}