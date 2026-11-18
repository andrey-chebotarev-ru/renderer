#pragma once

#include "BrushMap.h"

#include <charconv>

std::string BrushEntity::getProperty(const std::string& propName) const
{
	auto found = properties.find(propName);

	if (found == properties.end()) {
		return "";
	}

	return found->second;
}

bool BrushEntity::parseVec(Vec3& vec, const std::string& propName) const
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

	return true;
}