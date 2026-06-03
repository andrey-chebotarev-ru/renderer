#pragma once

#include <vector>

#include "Vector.h"
#include "Model.h"
#include "Math.h"

class Constructor
{
public:
	static Triangle3DList room(const Vec2& uv, const Vec3& size, int texId);
	static Triangle3DList cube(const Vec2& uv, const Vec3& size, int texId);
	static Poly3DList polyCube(const Vec3& size);

	static Poly3DList wall(const Vec2& vec1, const Vec2& vec2, float height);
	static FaceList map(const Vec2List list, float height, int wallTexId, int floorTexId, int ceilTexId);

	static void insert(FaceList& faces, const Poly3DList& polys, int texIdx, const Vec3& pos = Vec3(0, 0, 0), const Vec3& rot = Vec3(0, 0, 0));
};