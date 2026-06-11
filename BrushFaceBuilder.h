#pragma once

#include <unordered_map>
#include <string>

#include "Triangle.h"
#include "Map.h"
#include "Texture.h"

class BrushFaceBuilder
{
public:
	void build(FaceList& out, const MapBrush& brush, const TextureTable& table);
	void build(FaceList& out, Vec3List& playerStart, const Map& map, const TextureTable& table);
};

