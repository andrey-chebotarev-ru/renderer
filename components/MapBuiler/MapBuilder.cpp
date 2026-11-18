#include "MapBuilder.h"

#include <SDL3/SDL.h>

#include "../../Geometry.h"
#include "../../Debug.h"
#include "../../Texture.h"
#include "BrushMap.h"

void MapBuilder::buildFaces(FaceList& faces, const Brush& brush, TextureTable& table) const
{
	for (int sideIdx = 0; sideIdx < brush.sides.size(); sideIdx++) {

		const BrushSide& side = brush.sides[sideIdx];
		const Plane sidePlane = Geometry::makePlane(side.p1, side.p2, side.p3);

		PlaneList clipPlanes;

		for (int i = 0; i < brush.sides.size(); i++) {

			if (sideIdx == i) {
				continue;
			}

			const BrushSide& clipSide = brush.sides[i];
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
		}

		std::reverse(poly.verts.begin(), poly.verts.end());

		FaceType type = FaceType::Wall;

		if (std::string(side.texName).starts_with("*") || std::string(side.texName).starts_with("!")) {
			type = FaceType::Liquid;
		}

		uint32_t texIdx = table.add(side.texName);
		faces.push_back(Face(poly, texIdx, type));
	}
}

Vec3 MapBuilder::convert(const Vec3& vec) const
{
	return Vec3(vec.x, -vec.z, vec.y) / 5.0f;
}

bool MapBuilder::buildMap(Map& map, const std::string& filename) const
{
	Parser parser;
	BrushMap brushMap;
	TextureTable table;
	EntityList entities;
	Tree tree;
	Vec3List playerStart;
	PVSTable pvsTable;

	BSP bsp(tree);
	PVS pvs(tree);

	if (!parser.load(filename)) {
		return false;
	}

	parser.parse(brushMap);

	if (!brushMap.entities.size()) {
		Debug::print("No entities found");
		return false;
	}

	if (brushMap.entities[0].getProperty("classname") != "worldspawn") {
		Debug::print("Very first entity must have a worldspawn classname");
		return false;
	}

	for (const auto& brushEntity : brushMap.entities) {

		FaceList faces;
		EntityProperties props;

		for (const auto& brushes : brushEntity.brushes) {
			buildFaces(faces, brushes, table);
		}

		entities.push_back(Entity(brushEntity.properties, faces));
	}

	for (const auto& entity : entities) {
		if (entity.getClassname() == "info_player_start") {
			Vec3 startVec;
			if (entity.parseVec(startVec, "origin")) {
				playerStart.push_back(startVec);
			}
		}
	}

	if (!playerStart.size()) {
		Debug::print("Player start not found");
		return false;
	}

	if (bsp.compile(entities[0].faces, playerStart)) {
		Debug::print("Leak detected");
		return false;
	}

	pvs.calculate(pvsTable);

	map.textureNames = table.getNames();
	map.tree = tree;
	map.pvsTable = std::move(pvsTable);
	map.playerStart = std::move(playerStart);

	for (int i = 1; i < entities.size(); i++) {
		map.entities.push_back(entities[i]);
	}

	Debug::print("Compilation completed");

	return true;
}