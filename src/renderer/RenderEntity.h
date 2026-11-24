#pragma once

#include <vector>

#include "common/Common.h"
#include "common/Entity.h"

struct RenderEntity
{
	Entity entity;
	IndicesList touchedLeaves;
};

using RenderEntityList = std::vector<RenderEntity>;