#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Vector.h"

struct MapBrushSide
{
	Vec3 p1, p2, p3;

	std::string texName;

	Vec3 uAxis;
	float uShift;

	Vec3 vAxis;
	float vShift;

	float rotation;

	float xScale;
	float yScale;
};

using MapBrushSideList = std::vector<MapBrushSide>;

struct MapBrush
{
	MapBrushSideList sides;
};

using MapBrushList = std::vector<MapBrush>;

using MapEntityProperties = std::unordered_map<std::string, std::string>;

struct MapEntity
{
	MapEntityProperties properties;
	MapBrushList brushes;
};

using MapEntityList = std::vector<MapEntity>;

struct Map
{
	MapEntityList entities;
};