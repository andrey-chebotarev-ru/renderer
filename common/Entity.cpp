#pragma once

#include "Entity.h"

#include <charconv>

Entity::Entity() {
}

Entity::Entity(const EntityProperties& properties, const FaceList& faces)
	: properties(properties), faces(faces)
{
	for (auto& face : this->faces) {
		for (auto& vert : face.poly.verts) {
			vert.pos = convert(vert.pos);
		}
	}
}

std::string Entity::getProperty(const std::string& name) const
{
	auto found = properties.find(name);

	if (found == properties.end()) {
		return "";
	}

	return found->second;
}

std::string Entity::getClassname() const
{
	return getProperty("classname");
}

bool Entity::parseVec(Vec3& vec, const std::string& propName) const
{
	std::string value = getProperty(propName);

	if (!value.length()) {
		return false;
	}

	const char* ptr = value.data();
	const char* end = ptr + value.size();

	std::from_chars(ptr, end, vec.x);

	while (*ptr != ' ') {
		++ptr;
	}

	++ptr;

	std::from_chars(ptr, end, vec.y);

	while (*ptr != ' ') {
		++ptr;
	}

	++ptr;

	std::from_chars(ptr, end, vec.z);

	vec = convert(vec);

	return true;
}

Vec3 Entity::convert(const Vec3& vec) const
{
	return Vec3(vec.x, -vec.z, vec.y) / 5.0f;
}