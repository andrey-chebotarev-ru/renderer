#pragma once

#include "Map.h"

#include "Debug.h"

Map::Map() {
}

Map::Map(const BSP::Tree& tree, const PVSTable& pvsTable, const StringList& textureNames, const EntityList& entities)
	: tree(tree), pvsTable(pvsTable), textureNames(textureNames), entities(entities) {
}

const Entity& Map::getEntity(const std::string classname)
{
	for (const auto& entity : entities) {
		if (entity.getClassname() == classname) {
			return entity;
		}
	}

	Debug::printf("Could not find entity '{}'", classname);

	return Entity();
}