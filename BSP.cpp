#include "BSP.h"

#include <iostream>
#include <cstdlib>

#include <SDL3/SDL.h>



static Color changeColor(const Color& c, int j)
{
	Color out = c;
	if (out.r >= 128) {
		out.r -= j;
	}
	else {
		out.r += j;
	}

	if (out.g >= 128) {
		out.g -= j;
	}
	else {
		out.g += j;
	}

	if (out.b >= 128) {
		out.b -= j;
	}
	else {
		out.b += j;
	}

	return out;
}

Splitter BSP::findSplitter(const Poly3DList& polys)
{
	Splitter splitter = {
		.valid = false,
		.idx = -1
	};

	if (polys.size() <= 1) {
		return splitter;
	}

	int minScore = polys.size() * 20; // FIX

	for (int j = 0; j < polys.size(); j++) {
		const Poly3D& splitterPoly = polys[j];
		Plane splitterPlane = splitterPoly.getPlane();

		int frontCount = 0, backCount = 0, splitCount = 0;

		for (int i = 0; i < polys.size(); i++) {
			if (i == j) {
				continue;
			}

			
			SplittedPoly3D splitted = Geometry::splitAgainstPlane(polys[i], splitterPlane);

			if (splitted.result == SplitResult3D::Front) {
				frontCount++;
			}
			else if (splitted.result == SplitResult3D::Back) {
				backCount++;
			}
			else if (splitted.result == SplitResult3D::Intersected) {
				frontCount++;
				backCount++;
				splitCount++;
			}
		}

		if (frontCount > 0 && backCount > 0) {
			int score = splitCount * 16 + abs(backCount - frontCount);
			if (splitterPoly.getPlane().isAxial()) {
				score -= 4;
			}
			if (score < minScore) {
				splitter.valid = true;
				splitter.idx = j;
				splitter.plane = splitterPlane;
				minScore = score;
			}
		}
	}

	return splitter;
}

int BSP::createNode()
{
	nodes.push_back(Node());
	return nodes.size() - 1;
}

int BSP::addWorldPoly(const Poly3D& poly, int frontLeafIdx, int backLeafIdx)
{
	WorldPoly worldPoly = WorldPoly(poly);
	
	worldPolys.push_back(worldPoly);

	return worldPolys.size() - 1;
}

int BSP::addLeaf(const Leaf& leaf)
{
	leaves.push_back(leaf);
	return leaves.size() - 1;
}

int BSP::createPortal(const Poly3D& poly, int frontIndex, int backIndex)
{
	Portal portal;
	
	portal.valid = true;
	portal.poly = poly;
	portal.frontIndex = frontIndex;
	portal.backIndex = backIndex;
	portal.poly.fill = Color();

	portals.push_back(portal);

	return portals.size() - 1;
}

void BSP::compile(const Poly3DList& polys, const Vec3& start)
{
	nodes.clear();
	worldPolys.clear();
	portalLabels.clear();
	portalPolys.clear();

	BoundingBox worldBBox = makeBoundingBox(polys);
	WorldPolyList worldBounds = makeWorldBounds(worldBBox, 10.0f);

	Poly3DList p2 = polys;

	for (const auto& p : worldBounds) {
		p2.push_back(p.poly);
	}


	Context ctx;
	ctx.bounds = getPlanes(worldBBox);

	rootNodeIndex = createNode();
	ctx.nodeIdx = rootNodeIndex;

	build(p2, ctx);



	for (int leafIdx = 0; leafIdx < leaves.size(); leafIdx++) {

		PlaneList leafBounds;

		for (const auto& leafPolyIdx : leaf(leafIdx).indices) {
			leafBounds.push_back(worldPolys[leafPolyIdx].poly.getPlane());
		}

		for (int portalIdx = 0; portalIdx < portals.size(); portalIdx++) {

			

			if (portal(portalIdx).valid && node(portal(portalIdx).frontIndex).leafIndex == leafIdx || node(portal(portalIdx).backIndex).leafIndex == leafIdx) {
				
				Poly3D clippedPortal = clipPolygon(portal(portalIdx).poly, leafBounds);

				if (!clippedPortal.isValid()) {
			
					portal(portalIdx).valid = false;
				}
			}


		}
		

		


	}



	floodFill(getLeafIdx(start));
}

void BSP::build(const Poly3DList& polys, const Context& ctx)
{
	if (polys.size() == 0) {
		return;
	}

	int nodeIdx = ctx.nodeIdx;
	Splitter splitter = findSplitter(polys);

	if (splitter.valid) {

		int frontNodeIdx = createNode();
		int backNodeIdx = createNode();

		node(nodeIdx).frontIndex = frontNodeIdx;
		node(nodeIdx).backIndex = backNodeIdx;

		Poly3DList front, back, coplanar;
		Context frontCtx, backCtx;

		node(nodeIdx).leafIndex = -1;
		node(nodeIdx).plane = splitter.plane;
		
		// Разбиваем полигоны сплиттером
		for (int i = 0; i < polys.size(); i++) {

			
			SplittedPoly3D splittedPoly = Geometry::splitAgainstPlane(polys[i], splitter.plane);

			if (splittedPoly.result == SplitResult3D::Coplanar) {
				Poly3D coplanarPoly = splittedPoly.coplanar;
				coplanarPoly.fill = Color(255, 255, 255);
				coplanar.push_back(coplanarPoly);

				front.push_back(coplanarPoly);
				back.push_back(coplanarPoly);
			}
			else if (splittedPoly.result == SplitResult3D::Front) {
				front.push_back(splittedPoly.front);
			}
			else if (splittedPoly.result == SplitResult3D::Back) {
				back.push_back(splittedPoly.back);
			}
			else if (splittedPoly.result == SplitResult3D::Intersected) {
				front.push_back(splittedPoly.front);
				back.push_back(splittedPoly.back);
			}
		}

		/*
		* 
		* Портал является границей между листьями BSP-дерева - выпуклого объема и лежит на плоскости сплиттера
		* При создании он ссылается на дочерние ноды, которые еще не обработаны
		* Следовательно, эти ноды не являются финальными и могут в дальнейшем разделять пространство,
		* поэтому нужно 

		*  
		*/

		// Пройдемся по всем порталам которые создала родительская нода и попробуем их разделить плоскостью сплиттера
		// Классифицируем и обновим его 
		for (int portalIdx : node(nodeIdx).portals) {
			SplittedPoly3D splittedPortalPoly = Geometry::splitAgainstPlane(portal(portalIdx).poly, splitter.plane);

			portal(portalIdx).valid = false;
			int otherIdx = portal(portalIdx).frontIndex == nodeIdx ? portal(portalIdx).backIndex : portal(portalIdx).frontIndex;
			
			if (splittedPortalPoly.front.isValid()) {
				int portalIdx = createPortal(splittedPortalPoly.front, frontNodeIdx, otherIdx);
				node(frontNodeIdx).portals.push_back(portalIdx);
				node(otherIdx).portals.push_back(portalIdx);
			}

			if (splittedPortalPoly.back.isValid()) {
				int portalIdx = createPortal(splittedPortalPoly.back, backNodeIdx, otherIdx);
				node(backNodeIdx).portals.push_back(portalIdx);
				node(otherIdx).portals.push_back(portalIdx);
			}

			// В теории невозможно
			if (splittedPortalPoly.coplanar.isValid()) {
				int portalIdx = createPortal(splittedPortalPoly.coplanar, frontNodeIdx, otherIdx);
				node(frontNodeIdx).portals.push_back(portalIdx);
				node(otherIdx).portals.push_back(portalIdx);
			}

		}

		Poly3DList portalPolys = createPortalPolys(splitter.plane, ctx.bounds, coplanar);

		// Новый портал сразу попадает в оба списка дочерних узлов для возможного будущего разделения
		for (const auto& portalPoly : portalPolys) {
			int portalIdx = createPortal(portalPoly, frontNodeIdx, backNodeIdx);
			node(frontNodeIdx).portals.push_back(portalIdx);
			node(backNodeIdx).portals.push_back(portalIdx);
		}

		// Формируем bounds для дочерних элементов
		frontCtx.nodeIdx = frontNodeIdx;
		frontCtx.bounds = ctx.bounds;
		frontCtx.bounds.push_back(nodes[nodeIdx].plane);

		backCtx.nodeIdx = backNodeIdx;
		backCtx.bounds = ctx.bounds;
		backCtx.bounds.push_back(splitter.plane.flip());

		build(front, frontCtx);
		build(back, backCtx);
	}
	else {
		Leaf leaf;

		nodes[nodeIdx].frontIndex = -1;
		nodes[nodeIdx].backIndex = -1;

		PlaneList leafBounds;

		Color c;
		for (int j = 0; j < polys.size(); j++) {
			Poly3D poly = polys[j];

			leafBounds.push_back(poly.getPlane());


			poly.fill = changeColor(c, j);
			leaf.indices.push_back(addWorldPoly(poly, -1, -1));
		}

		nodes[nodeIdx].leafIndex = addLeaf(leaf);
	}
}

void BSP::render(const Vec3& pos, World& world, bool populateLeafs, bool renderOnlyCurrentLeaf)
{
	if (renderOnlyCurrentLeaf) {
		int currentLeafIdx = getLeafIdx(pos);
		Leaf leaf2 = leaf(currentLeafIdx);

		if (leaf2.valid) {
			for (int index : leaf2.indices) {
				world.addGeometry(worldPolys[index].poly);
			}
		}
	}
	else {
		traverse(rootNodeIndex, pos, world);
	}

	portalPolys.clear();
	if (populateLeafs) {
		int currentLeafIdx = getLeafIdx(pos);
		visibleLabels.clear();

		int i = 0;
		for (const auto& portal : portals) {
			if (portal.valid && node(portal.frontIndex).leafIndex != -1 && node(portal.backIndex).leafIndex != -1) {
				int leafA = node(portal.frontIndex).leafIndex;
				int leafB = node(portal.backIndex).leafIndex;
				if (leafA == currentLeafIdx || leafB == currentLeafIdx) {
					if (leafA != currentLeafIdx) {
						std::swap(leafA, leafB);
					}
					static char buf[256];
					sprintf_s(buf, "#%i %i <=> %i", i, leafA, leafB);
					visibleLabels.push_back(Label(portal.poly.getCenter(), Color(255, 0, 0), buf));
					portalPolys.push_back(portal.poly);
				}
			}
			i++;
		}
	}
}

void BSP::traverse(int nodeIndex, const Vec3& pos, World& world)
{
	if (nodeIndex == -1) {
		return;
	}

	const Node& node = nodes[nodeIndex];

	if (node.leafIndex != -1)
	{
		Leaf leaf = leaves[node.leafIndex];

		if (leaf.valid) {
			for (int index : leaf.indices) {
				world.addGeometry(worldPolys[index].poly);
			}
		}
	}
	else
	{
		Side side = Geometry::test(node.plane, pos);

		if (side == Side::Front) {
			traverse(node.backIndex, pos, world);
			traverse(node.frontIndex, pos, world);
		}
		else if (side == Side::Back) {
			traverse(node.frontIndex, pos, world);
			traverse(node.backIndex, pos, world);
		}
		else {
			traverse(node.frontIndex, pos, world);
			traverse(node.backIndex, pos, world);
		}
	}
}

int BSP::getLeafIdx(const Vec3& vec)
{
	int nodeId = rootNodeIndex;
	while (true) {
		const Node& node = nodes[nodeId];

		if (node.leafIndex != -1) {
			return node.leafIndex;
		}

		if (Geometry::test(node.plane, vec) == Side::Back) {
			nodeId = node.backIndex;
		}
		else {
			nodeId = node.frontIndex;
		}
	}

	return 0;
}

// AI
Poly3D BSP::makePolygonOnPlane(const Plane& plane)
{
	Vec3 center = plane.normal * plane.distance;

	Vec3 temp = fabs(plane.normal.y) < 0.99f ? Vec3(0, 1, 0) : Vec3(1, 0, 0);

	Vec3 axisU = Vec3::normalize(Vec3::cross(temp, plane.normal));
	Vec3 axisV = Vec3::normalize(Vec3::cross(plane.normal, axisU));

	constexpr float size = 1000.0f;

	Poly3D poly;

	poly.texId = 1;
	poly.fill = Color();

	poly.verts.push_back(Vertex3D(center - axisU * size - axisV * size, Vec2(0, 0)));
	poly.verts.push_back(Vertex3D(center + axisU * size - axisV * size, Vec2(1, 0)));
	poly.verts.push_back(Vertex3D(center + axisU * size + axisV * size, Vec2(1, 1)));
	poly.verts.push_back(Vertex3D(center - axisU * size + axisV * size, Vec2(0, 1)));

	return poly;
}

Poly3D BSP::clipPolygon(const Poly3D& in, const PlaneList& bounds)
{
	Poly3D out = in;
	
	for (const Plane& plane : bounds)
	{
		SplittedPoly3D splittedPoly = Geometry::splitAgainstPlane(out, plane);

		if (splittedPoly.result == SplitResult3D::Front || splittedPoly.result == SplitResult3D::Intersected) {
			out = splittedPoly.front;
		}
		else if (splittedPoly.result == SplitResult3D::Back) {
			out.verts.clear();
			return out;
		}
	}

	return out;
}

BoundingBox BSP::makeBoundingBox(const Poly3DList& polys)
{
	BoundingBox bbox;

	if (polys.size() == 0) {
		return bbox;
	}

	// FIX
	bbox.min = bbox.max = polys[0].verts[0].position;

	for (const auto& poly : polys) {

		for (const auto& vert : poly.verts) {
			bbox.min.x = std::min(bbox.min.x, vert.position.x);
			bbox.min.y = std::min(bbox.min.y, vert.position.y);
			bbox.min.z = std::min(bbox.min.z, vert.position.z);

			bbox.max.x = std::max(bbox.max.x, vert.position.x);
			bbox.max.y = std::max(bbox.max.y, vert.position.y);
			bbox.max.z = std::max(bbox.max.z, vert.position.z);
		}
	}

	return bbox;
}

PlaneList BSP::getPlanes(const BoundingBox& bbox)
{
	PlaneList planes;

	planes.push_back(Plane(Vec3(-1, 0, 0), -bbox.max.x));
	planes.push_back(Plane(Vec3(1, 0, 0), bbox.min.x));

	planes.push_back(Plane(Vec3(0, -1, 0), -bbox.max.y));
	planes.push_back(Plane(Vec3(0, 1, 0), bbox.min.y));

	planes.push_back(Plane(Vec3(0, 0, -1), -bbox.max.z));
	planes.push_back(Plane(Vec3(0, 0, 1), bbox.min.z));

	return planes;
}

static Vec3 anyPerpendicular(const Vec3& n)
{
	if (fabs(n.x) < fabs(n.y) && fabs(n.x) < fabs(n.z)) {
		return Vec3::normalize(Vec3::cross(n, Vec3(1, 0, 0)));
	}
	else if (fabs(n.y) < fabs(n.z)) {
		return Vec3::normalize(Vec3::cross(n, Vec3(0, 1, 0)));
	}
	else {
		return Vec3::normalize(Vec3::cross(n, Vec3(0, 0, 1)));
	}
}

static Vec2 projectTo2D(const Vec3& p, const Vec3& origin, const Vec3& u, const Vec3& v)
{
	Vec3 q = p - origin;

	return Vec2(Vec3::dot(q, u), Vec3::dot(q, v));
}

static Vec3 unprojectTo3D(const Vec2& p, const Vec3& origin, const Vec3& u, const Vec3& v)
{
	return origin + u * p.x + v * p.y;
}

Poly3DList BSP::createPortalPolys(const Plane& plane, const PlaneList& bounds, const Poly3DList& coplanarPolys)
{
	Poly3DList portalPolys;
	Poly2DList portalPolys2D;

	// Создаем портал используя плоскость сплиттера
	Poly3D portalPoly = makePolygonOnPlane(plane);
	portalPoly = clipPolygon(portalPoly, bounds);

	// Подготовка
	Vec3 origin = plane.normal * plane.distance;
	Vec3 u = Vec3::normalize(anyPerpendicular(plane.normal));
	Vec3 v = Vec3::cross(plane.normal, u);

	Poly2D portalPoly2D;
	
	for (int i = 0; i < portalPoly.getSize(); i++) {
		Vec2 proj = projectTo2D(portalPoly.verts[i].position, origin, u, v);
		portalPoly2D.verts.push_back({ proj, 1, 1, 1});
	}

	portalPolys2D.push_back(portalPoly2D);

	for (const auto& coplanarPoly : coplanarPolys)
	{
		Poly2D coplanarPoly2D;

		for (int i = 0; i < coplanarPoly.getSize(); i++) {
			Vec2 proj = projectTo2D(coplanarPoly.verts[i].position, origin, u, v);
			coplanarPoly2D.verts.push_back({ proj, 1, 1, 1 });
		}

		Poly2DList preservedPolys2D = portalPolys2D;
		portalPolys2D.clear();
		for (const auto& poly2D : preservedPolys2D) {
			auto result = Geometry::subtract(poly2D, coplanarPoly2D, 50.0f);
			portalPolys2D.insert(portalPolys2D.end(), result.begin(), result.end());
		}
	}

	for (const auto& portalPoly2D : portalPolys2D) {
		Poly3D portalPoly;
		portalPoly.fill = Color();
		portalPoly.texId = 1;
		for (int i = 0; i < portalPoly2D.getSize(); i++) {
			portalPoly.verts.push_back(Vertex3D(unprojectTo3D(portalPoly2D.verts[i].pos, origin, u, v), Vec2(1, 1)));
		}

		portalPolys.push_back(portalPoly);
	}

	return portalPolys;
}

void BSP::floodFill(int leafIdx)
{
	for (int i = 0; i < portals.size(); i++) {
		if (portal(i).valid && !portal(i).visited && node(portal(i).frontIndex).leafIndex != -1 && node(portal(i).backIndex).leafIndex != -1) {
			int currentLeafIdx = node(portal(i).frontIndex).leafIndex;
			int nextLeafIdx = node(portal(i).backIndex).leafIndex;

			if (currentLeafIdx == leafIdx || nextLeafIdx == leafIdx) {
				
				if (currentLeafIdx != leafIdx) {
					std::swap(currentLeafIdx, nextLeafIdx);
				}
				leaves[currentLeafIdx].valid = true;
				leaves[nextLeafIdx].valid = true;
				portal(i).visited = true;
				floodFill(nextLeafIdx);
			}
		}
	}
}

WorldPolyList BSP::makeWorldBounds(const BoundingBox& src, float padding)
{
	WorldPolyList polys;
	BoundingBox bbox = src;

	bbox.min -= padding;
	bbox.max += padding;

	Vec3 v000 = Vec3(bbox.min.x, bbox.min.y, bbox.min.z);
	Vec3 v001 = Vec3(bbox.min.x, bbox.min.y, bbox.max.z);
	Vec3 v010 = Vec3(bbox.min.x, bbox.max.y, bbox.min.z);
	Vec3 v011 = Vec3(bbox.min.x, bbox.max.y, bbox.max.z);
	
	Vec3 v100 = Vec3(bbox.max.x, bbox.min.y, bbox.min.z);
	Vec3 v101 = Vec3(bbox.max.x, bbox.min.y, bbox.max.z);
	Vec3 v110 = Vec3(bbox.max.x, bbox.max.y, bbox.min.z);
	Vec3 v111 = Vec3(bbox.max.x, bbox.max.y, bbox.max.z);

	auto addPoly = [&](const Vec3& vec1, const Vec3& vec2, const Vec3& vec3, const Vec3& vec4) {
		polys.push_back(WorldPoly(Poly3D({ {vec1, {0, 0}}, {vec2, {0, 1}}, {vec3, {1, 1}}, {vec4, {1, 0}} }, 5), WorldPolyType::WorldPolyBounds));
	};

	addPoly(v000, v001, v011, v010);
	addPoly(v100, v110, v111, v101);
	addPoly(v000, v100, v101, v001);
	addPoly(v010, v011, v111, v110);
	addPoly(v000, v010, v110, v100);
	addPoly(v001, v101, v111, v011);

	return polys;
}