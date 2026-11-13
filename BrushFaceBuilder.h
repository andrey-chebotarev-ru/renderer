#pragma once

#include <unordered_map>
#include <string>

#include "Common.h"
#include "Triangle.h"
#include "Map.h"
#include "Texture.h"


class BrushFaceBuilder
{
public:
	void build(FaceList& out, const MapBrush& brush, TextureTable& table);
	StringList build(FaceList& out, Vec3List& playerStart, const Map& map);
};

