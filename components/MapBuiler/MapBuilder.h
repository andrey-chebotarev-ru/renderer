#pragma once

#include <unordered_map>
#include <string>

#include "../../common/Common.h"
#include "../../common/Entity.h"
#include "../../common/Map.h"
#include "../../Texture.h"
#include "../../Triangle.h"

#include "Parser.h"
#include "BrushMap.h"

class MapBuilder
{
public:
	void buildFaces(FaceList& faces, const Brush& brush, TextureTable& table) const;
	bool buildMap(Map& map, const std::string& filename) const;
	Vec3 convert(const Vec3& vec) const;
};