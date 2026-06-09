#pragma once

#include <unordered_map>
#include <string>

#include "Triangle.h"
#include "Map.h"

class BrushFaceBuilder
{
public:
	static std::unordered_map<std::string, size_t> textures;

	static void build(FaceList& out, const MapBrush& brush);
	static void build(FaceList& out, const MapBrushList& brushes);
	static void build(FaceList& out, Vec3& start, const Map& map);
	static size_t getTextureId(std::string texName);
};

