#pragma once

#include <string>

#include "common/Common.h"
#include "common/Entity.h"
#include "renderer/BSP.h"
#include "renderer/PVS.h";

class Map
{
public:

	Map();
	Map(const BSP::Tree& tree, const PVSTable& pvsTable, const StringList& textureNames, const EntityList& entities);

	BSP::Tree tree;
	PVSTable pvsTable;
	StringList textureNames;
	EntityList entities;
	Vec3List playerStart;
	
	const Entity& getEntity(const std::string classname);
};