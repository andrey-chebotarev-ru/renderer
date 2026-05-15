#pragma once

#include "Vector.h"
#include "Model.h"

class Constructor
{
public:
	static Model room(const Vec3& position, const Vec3& rotation, const Vec2& uv, const Vec3& size, size_t textureIndex);
	static Model cube(const Vec3& position, const Vec3& rotation, const Vec3& size, size_t textureIndex);
	static Model test(const Vec3& position, const Vec3& rotation, size_t textureIndex);
};