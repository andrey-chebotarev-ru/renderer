#pragma once

#include <vector>

#include "Vector.h"
#include "Model.h"
#include "Math.h"

class Constructor
{
public:
	static Triangle3DList room(const Vec2& uv, const Vec3& size, size_t texId);
	static Triangle3DList cube(const Vec2& uv, const Vec3& size, size_t texId);
	static Poly3DList polyCube(const Vec2& uv, const Vec3& size, const Transform& tr, size_t texId);

	static Poly3DList wall(const Vec2& vec1, const Vec2& vec2, float height, size_t texId);
	static Poly3DList map(const Vec2List list, float height, size_t texId);
};