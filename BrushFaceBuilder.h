#pragma once

#include <unordered_map>
#include <string>

#include "Triangle.h"
#include "Map.h"
#include "Texture.h"

class BrushFaceBuilder
{
public:

	TextureManager& manager;

	BrushFaceBuilder(TextureManager& manager)
		: manager(manager) {
	}

	std::unordered_map<std::string, size_t> textures;

	void build(FaceList& out, const MapBrush& brush);
	void build(FaceList& out, const MapBrushList& brushes);
	void build(FaceList& out, Vec3& start, const Map& map);
	size_t getTextureId(std::string texName);
};

