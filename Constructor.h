#pragma once

#include <vector>
#include <string>

#include "Vector.h"
#include "Model.h"
#include "Math.h"
#include "Texture.h"

class Constructor
{
public:
	Constructor(const TextureManager& textureManager)
		: textureManager(textureManager) {
	}

	Triangle3DList room(const Vec2& uv, const Vec3& size, const std::string& texName) const;
	Triangle3DList cube(const Vec2& uv, const Vec3& size, const std::string& texName) const;
	Poly3DList polyCube(const Vec3& size) const;

	Poly3DList wall(const Vec2& vec1, const Vec2& vec2, float height) const;
	FaceList map(const Vec2List list, float height, const std::string& wallTexName, const std::string& floorTexName, const std::string& ceilTexName) const;

	void insert(FaceList& faces, const Poly3DList& polys, size_t texIdx, const Vec3& pos = Vec3(0, 0, 0), const Vec3& rot = Vec3(0, 0, 0)) const;
	void insert(FaceList& faces, const Poly3DList& polys, const std::string& texName, const Vec3& pos = Vec3(0, 0, 0), const Vec3& rot = Vec3(0, 0, 0)) const;

private:

	const TextureManager& textureManager;
};