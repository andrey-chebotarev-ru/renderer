#pragma once

#include <vector>

#include "Vector.h"
#include "Face.h"

namespace Bounds
{
	struct AABB2
	{
		Vec2 min;
		Vec2 max;
	};

	struct AABB3
	{
		Vec3 min;
		Vec3 max;
	};

	using AABB3List = std::vector<AABB3>;

	AABB3 create(const AABB3List& aabbs);
	AABB3 create(const FaceList& faces);
}