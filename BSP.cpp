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

int BSP::findSplitter(const Poly3DList& polys)
{
	int index = -1;

	if (polys.size() <= 1) {
		return index;
	}

	int minScore = polys.size() * 2;

	for (int j = 0; j < polys.size(); j++) {
		const Poly3D& splitter = polys[j];

		int frontCount = 0, backCount = 0;

		for (int i = 0; i < polys.size(); i++) {
			if (i == j) {
				continue;
			}

			SplittedPoly3D splitted = Geometry::splitAgainstPlane(polys[i], splitter.getPlane());

			if (splitted.result == SplitResult3D::Front) {
				frontCount++;
			}
			else if (splitted.result == SplitResult3D::Back) {
				backCount++;
			}
			else if (splitted.result == SplitResult3D::Intersected) {
				frontCount++;
				backCount++;
			}
		}

		if (frontCount > 0 && backCount > 0) {
			int score = abs((int)(backCount - frontCount));
			if (score <= minScore) {
				index = j;
				minScore = score;
			}
		}
	}

	return index;
}

int BSP::createNode()
{
	nodes.push_back(Node());
	return nodes.size() - 1;
}

int BSP::addToList(const Poly3D& poly)
{
	list.push_back(poly);
	return list.size() - 1;
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

	std::cout << "Created portal " << (portals.size() - 1) << ": " << frontIndex << " <=> " << backIndex << "\r\n";

	return portals.size() - 1;
}

void BSP::compile(const Poly3DList& polys, const Vec3& start)
{
	nodes.clear();
	list.clear();
	portalLabels.clear();
	portalPolys.clear();

	BoundingBox worldBBox = makeBoundingBox(polys);
	PlaneList bounds = getPlanes(worldBBox);

	rootNodeIndex = createNode();

	build(rootNodeIndex, polys, bounds);
	floodFill(leaf(start));
}

void BSP::render(const Vec3& pos, World& world, bool populateLeafs)
{
	traverse(rootNodeIndex, pos, world);
	portalPolys.clear();
	if (populateLeafs) {
		int currentLeafIdx = leaf(pos);
		visibleLabels.clear();
		
		for (const auto& portal : portals) {
			if (portal.valid && node(portal.frontIndex).leafIndex != -1 && node(portal.backIndex).leafIndex != -1) {
				int leafA = node(portal.frontIndex).leafIndex;
				int leafB = node(portal.backIndex).leafIndex;
				if (leafA == currentLeafIdx || leafB == currentLeafIdx) {
					if (leafA != currentLeafIdx) {
						std::swap(leafA, leafB);
					}
					static char buf[256];
					sprintf_s(buf, "%i <=> %i", leafA, leafB);
					visibleLabels.push_back(Label(portal.poly.getCenter(), Color(255, 0, 0), buf));
					portalPolys.push_back(Poly3D::offset(portal.poly, 0.1f));
				}
			}
		}
	}
}

void BSP::build(int nodeIdx, const Poly3DList& polys, const PlaneList& bounds)
{
	if (polys.size() == 0) {
		return;
	}

	int splitterIndex = findSplitter(polys);

	if (splitterIndex != -1) {

		int frontNodeIdx = createNode();
		int backNodeIdx = createNode();

		node(nodeIdx).frontIndex = frontNodeIdx;
		node(nodeIdx).backIndex = backNodeIdx;

		Poly3DList front, back;
		
		Poly3D splitter = polys[splitterIndex];
		Plane plane = splitter.getPlane();

		node(nodeIdx).leafIndex = -1;
		node(nodeIdx).plane = splitter.getPlane();
		splitter.fill = Color(255, 255, 255);
		node(nodeIdx).coplanar.push_back(addToList(splitter));


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
			SplittedPoly3D splittedPortalPoly = Geometry::splitAgainstPlane(portal(portalIdx).poly, splitter.getPlane());

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

		for (int i = 0; i < polys.size(); i++) {

			if (i == splitterIndex) {
				continue;
			}

			
			SplittedPoly3D splittedPoly = Geometry::splitAgainstPlane(polys[i], nodes[nodeIdx].plane);

			if (splittedPoly.result == SplitResult3D::Coplanar) {
				Poly3D coplanarPoly = splittedPoly.coplanar;
				coplanarPoly.fill = Color(255, 255, 255);
				node(nodeIdx).coplanar.push_back(addToList(coplanarPoly));
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


		Poly3DList portalPolys = createPortalPolys(plane, bounds, node(nodeIdx).coplanar);

		// Новый портал сразу попадает в оба списка дочерних узлов для возможного будущего разделения
		for (const auto& portalPoly : portalPolys) {
			int portalIdx = createPortal(portalPoly, frontNodeIdx, backNodeIdx);
			node(frontNodeIdx).portals.push_back(portalIdx);
			node(backNodeIdx).portals.push_back(portalIdx);
		}

		PlaneList frontBounds = bounds;
		frontBounds.push_back(nodes[nodeIdx].plane);

		PlaneList backBounds = bounds;
		backBounds.push_back(nodes[nodeIdx].plane.flip());

		build(frontNodeIdx, front, frontBounds);
		build(backNodeIdx, back, backBounds);
	}
	else {
		Leaf leaf;

		nodes[nodeIdx].frontIndex = -1;
		nodes[nodeIdx].backIndex = -1;

		Color c;
		for (int j = 0; j < polys.size(); j++) {
			Poly3D poly = polys[j];
			poly.fill = changeColor(c, j);
			leaf.indices.push_back(addToList(poly));
		}

		nodes[nodeIdx].leafIndex = addLeaf(leaf);
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
				world.addGeometry(list[index]);
			}
		}
	}
	else
	{
		Side side = Geometry::test(node.plane, pos);

		if (side == Side::Front) {
			traverse(node.backIndex, pos, world);

			for (int index : node.coplanar) {
				world.addGeometry(list[index]);
			}
			
			traverse(node.frontIndex, pos, world);
		}
		else if (side == Side::Back) {
			traverse(node.frontIndex, pos, world);

			for (int index : node.coplanar) {
				world.addGeometry(list[index]);
			}

			traverse(node.backIndex, pos, world);
		}
		else {
			traverse(node.frontIndex, pos, world);
			traverse(node.backIndex, pos, world);
		}

	}
}

int BSP::leaf(const Vec3& vec)
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

Poly3DList BSP::createPortalPolys(const Plane& plane, const PlaneList& bounds, const IndicesList coplanarIdxs)
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
		std::cout << "Portal v" << i << ": " << proj.x << " " << proj.y << "\r\n";
		portalPoly2D.verts.push_back({ proj, 1, 1, 1});
	}
	std::cout << "Portal 3D " << (portalPoly.getSignedArea() < 0 ? "CCW" : "CW") << "\r\n";
	std::cout << "Portal 2D " << (portalPoly2D.getSignedArea() < 0 ? "CCW" : "CW") << "\r\n";
	portalPolys2D.push_back(portalPoly2D);

	for (auto idx : coplanarIdxs)
	{
		Poly3D coplanarPoly = list[idx];
		Poly2D coplanarPoly2D;

		for (int i = 0; i < coplanarPoly.getSize(); i++) {
			Vec2 proj = projectTo2D(coplanarPoly.verts[i].position, origin, u, v);
			std::cout << "Polygon v" << i << ": " << proj.x << " " << proj.y << "\r\n";
			coplanarPoly2D.verts.push_back({ proj, 1, 1, 1 });
		}


		std::cout << "Poly 3D " << (coplanarPoly.getSignedArea() < 0 ? "CCW" : "CW") << "\r\n";
		std::cout << "Poly 2D " << (coplanarPoly2D.getSignedArea() < 0 ? "CCW" : "CW") << "\r\n";

		Poly2DList preservedPolys2D = portalPolys2D;
		portalPolys2D.clear();
		for (const auto& poly2D : preservedPolys2D) {
			auto result = Geometry::subtract(poly2D, coplanarPoly2D, 10.0f);
			portalPolys2D.insert(portalPolys2D.begin(), result.begin(), result.end());
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


	//portalPolys.push_back(portalPoly);

	return portalPolys;
}

void BSP::floodFill(int leafId)
{
	

	for (int i = 0; i < portals.size(); i++) {
		if (portal(i).valid && !portal(i).visited && node(portal(i).frontIndex).leafIndex != -1 && node(portal(i).backIndex).leafIndex != -1) {
			int currentLeafIdx = node(portal(i).frontIndex).leafIndex;
			int nextLeafIdx = node(portal(i).backIndex).leafIndex;

			if (currentLeafIdx == leafId || nextLeafIdx == leafId) {
				if (currentLeafIdx != leafId) {
					std::swap(currentLeafIdx, nextLeafIdx);
				}
				leaves[currentLeafIdx].valid = true;
				portal(i).visited = true;
				floodFill(nextLeafIdx);
			}
		}
	}

}