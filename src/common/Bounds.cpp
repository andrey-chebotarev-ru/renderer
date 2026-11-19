#include "Bounds.h"

#include <assert.h>

namespace Bounds
{
	/*
	* Объеденение нескольких AABB в один
	*/
	AABB3 create(const AABB3List& aabbs)
	{
		assert(aabbs.size() > 0);

		AABB3 out = aabbs[0];

		for (const auto& aabb : aabbs) {
			out.min.x = std::min(out.min.x, aabb.min.x);
			out.min.y = std::min(out.min.y, aabb.min.y);
			out.min.z = std::min(out.min.z, aabb.min.z);

			out.max.x = std::max(out.max.x, aabb.max.x);
			out.max.y = std::max(out.max.y, aabb.max.y);
			out.max.z = std::max(out.max.z, aabb.max.z);
		}

		return out;
	}

	/*
	* Создание ограничивающего AABB из списка сторон
	*/
	AABB3 create(const FaceList& faces)
	{
		AABB3 out;

		assert(faces.size() > 0);
		assert(faces[0].poly.verts.size() > 0);

		out.min = out.max = faces[0].poly.verts[0].pos;

		for (const auto& face : faces) {

			for (const auto& vert : face.poly.verts) {
				out.min.x = std::min(out.min.x, vert.pos.x);
				out.min.y = std::min(out.min.y, vert.pos.y);
				out.min.z = std::min(out.min.z, vert.pos.z);

				out.max.x = std::max(out.max.x, vert.pos.x);
				out.max.y = std::max(out.max.y, vert.pos.y);
				out.max.z = std::max(out.max.z, vert.pos.z);
			}
		}

		return out;
	}
}