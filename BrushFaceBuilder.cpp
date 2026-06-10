#include "BrushFaceBuilder.h"

#include <iostream>
#include <format>

#include <SDL3/SDL.h>

#include "Geometry.h"

static Vec3 convert(const Vec3& vec)
{
	return Vec3(vec.x, -vec.z, vec.y) / 5;
}

void BrushFaceBuilder::build(FaceList& out, const MapBrush& brush)
{
	for (int j = 0; j < brush.sides.size(); j++) {

		const MapBrushSide& side = brush.sides[j];
		const Plane sidePlane = Geometry::makePlane(side.p1, side.p2, side.p3);

		PlaneList clipPlanes;

		for (int i = 0; i < brush.sides.size(); i++) {

			if (j == i) {
				continue;
			}

			const MapBrushSide& clipSide = brush.sides[i];
			const Plane clipPlane = Geometry::makePlane(clipSide.p1, clipSide.p2, clipSide.p3);

			clipPlanes.push_back(clipPlane);
		}

		Poly3D poly = Geometry::makePolygonOnPlane(sidePlane, 100000.0f);

		

		poly = Geometry::clip(poly, clipPlanes);

		if (!poly.isValid()) {
			continue;
		}

		for (auto& vert : poly.verts) {

			float u = Vec3::dot(vert.pos, side.uAxis);
			float v = Vec3::dot(vert.pos, side.vAxis);

			u = u / side.xScale + side.uShift;
			v = v / side.yScale + side.vShift;

			vert.pos = convert(vert.pos);

			vert.uv.x = u;
			vert.uv.y = v;
		}

		std::reverse(poly.verts.begin(), poly.verts.end());

		out.push_back(Face(poly, manager.getIdx(side.texName)));
	}
}

void BrushFaceBuilder::build(FaceList& out, const MapBrushList& brushes)
{
	for (const auto& brush : brushes) {
		build(out, brush);
	}

	for (const auto& face : out) {
		std::cout << "Face:\n";
		for (int i = 0; i < face.poly.getCount(); i++) {
			const auto& vert = face.poly.verts[i];
			std::cout << std::format("Vert{}: ({} {} {})\n", i + 1, vert.pos.x, vert.pos.y, vert.pos.z);
		}
		
	}
}


static Vec3 parseOrigin(const std::string& str)
{
	Vec3 result;

	const char* ptr = str.data();
	const char* end = ptr + str.size();

	std::from_chars(ptr, end, result.x);

	while (*ptr != ' ') {
		++ptr;
	}

	++ptr;

	std::from_chars(ptr, end, result.y);

	while (*ptr != ' ') {
		++ptr;
	}

	++ptr;

	std::from_chars(ptr, end, result.z);


	return convert(result);
}

static std::string getProperty(const MapEntity& entity, const std::string& key)
{
	auto iter = entity.properties.find(key);

	if (iter == entity.properties.end()) {
		return "";
	}

	return iter->second;
}

void BrushFaceBuilder::build(FaceList& out, Vec3& start, const Map& map)
{
	for (const auto& entity : map.entities) {

		std::string classname = getProperty(entity, "classname");

		if (classname == "worldspawn") {
			for (const auto& brushes : entity.brushes) {
				build(out, brushes);
			}
		}
		else if (classname == "info_player_start") {
			start = parseOrigin(getProperty(entity, "origin"));
		}

	}
}

size_t BrushFaceBuilder::getTextureId(std::string texName)
{

	auto iter = textures.find(texName);

	if (iter == textures.end()) {
		size_t texId = SDL_rand(7);
		textures[texName] = texId;
		return texId;
	}
	else {
		return iter->second;
	}
}