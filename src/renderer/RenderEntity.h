#pragma once

#include <vector>

#include "common/Common.h"
#include "common/Entity.h"

struct RenderEntity
{
	FaceList faces;
	Bounds::AABB3 aabb;
	IndicesList touchedLeaves;
	uint32_t alpha = 255;
};

using RenderEntityList = std::vector<RenderEntity>;