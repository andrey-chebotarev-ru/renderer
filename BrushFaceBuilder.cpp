#include "BrushFaceBuilder.h"

#include <SDL3/SDL.h>

#include "Geometry.h"
#include "Debug.h"
#include "Texture.h"

static Vec3 convert(const Vec3& vec)
{
	return Vec3(vec.x, -vec.z, vec.y) / 5.0f;
}

void BrushFaceBuilder::build(FaceList& out, const MapBrush& brush, const TextureTable& table)
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

		Poly3D poly = Geometry::makePolygonOnPlane(sidePlane, 10000.0f);

		poly = Geometry::clip(poly, clipPlanes);

		if (!poly.isValid()) {
			continue;
		}

		for (auto& vert : poly.verts) {

			vert.uv.x = Vec3::dot(vert.pos, side.uAxis) / side.xScale + side.uShift;
			vert.uv.y = Vec3::dot(vert.pos, side.vAxis) / side.yScale + side.vShift;

			vert.pos = convert(vert.pos);
		}

		std::reverse(poly.verts.begin(), poly.verts.end());

		FaceType type = FaceType::Solid;

		if (std::string(side.texName).starts_with("*") || std::string(side.texName).starts_with("!")) {
			type = FaceType::Water;
		}

		out.push_back(Face(poly, table.find(side.texName), type));	
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

void BrushFaceBuilder::build(FaceList& out, Vec3List& playerStart, const Map& map, const TextureTable& table)
{
	for (const auto& entity : map.entities) {

		std::string classname = getProperty(entity, "classname");

		if (classname == "worldspawn") {
			for (const auto& brushes : entity.brushes) {
				build(out, brushes, table);
			}
		}
		else if (classname.starts_with("info_player_start")) {
			Vec3 origin = parseOrigin(getProperty(entity, "origin"));
			playerStart.push_back(origin);

			Debug::printf("Player start '{}' at ('{}', '{}', '{}')", classname, origin.x, origin.y, origin.z);
		}
	}
}