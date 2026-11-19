#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "Face.h"

using EntityProperties = std::unordered_map<std::string, std::string>;

class Entity
{
public:

	EntityProperties properties;
	FaceList faces;

	Entity();
	Entity(const EntityProperties& properties, const FaceList& faces);

	std::string getProperty(const std::string& name) const;
	std::string getClassname() const;

	bool parseVec(Vec3& vec, const std::string& propName) const;

	Vec3 convert(const Vec3& vec) const;
};

using EntityList = std::vector<Entity>;