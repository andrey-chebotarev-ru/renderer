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

		Vec2 getCenter() const {
			return (min + max) * 0.5f;
		}

		Vec2 getExtents() const {
			return (min - max) * 0.5f;
		}
	};

	struct AABB3
	{
		Vec3 min;
		Vec3 max;

		Vec3 getCenter() const {
			return (min + max) * 0.5f;
		}

		Vec3 getExtents() const {
			return (min - max) * 0.5f;
		}
	};

	using AABB3List = std::vector<AABB3>;

	AABB3 create(const AABB3List& aabbs);
	AABB3 create(const FaceList& faces);
}