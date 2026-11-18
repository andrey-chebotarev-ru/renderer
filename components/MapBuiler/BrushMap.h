#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../../Vector.h"

struct BrushSide
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

using BrushSideList = std::vector<BrushSide>;

struct Brush
{
	BrushSideList sides;
};

using BrushList = std::vector<Brush>;

using BrushEntityProperties = std::unordered_map<std::string, std::string>;

struct BrushEntity
{
	BrushEntityProperties properties;
	BrushList brushes;

	std::string getProperty(const std::string& propName) const;
	bool parseVec(Vec3& vec, const std::string& propName) const;
};

using BrushEntityList = std::vector<BrushEntity>;

struct BrushMap
{
	BrushEntityList entities;
};