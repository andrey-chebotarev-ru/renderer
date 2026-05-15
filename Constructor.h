#pragma once

#include "Vector.h"
#include "Model.h"

class Constructor
{
public:
	static Triangle3DList room(const Vec2& uv, const Vec3& size, size_t texId);
	static Triangle3DList cube(const Vec2& uv, const Vec3& size, size_t texId);
};