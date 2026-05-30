#include "BSP.h"

#include <iostream>
#include <cstdlib>

#include <SDL3/SDL.h>

Splitter BSP::findSplitter(const FaceList& faces)
{
	Splitter splitter = {
		.valid = false,
		.idx = -1
	};

	if (faces.size() <= 1) {
		return splitter;
	}

	int minScore = faces.size() * 20; // FIX

	for (int j = 0; j < faces.size(); j++) {
		const Face& splitterFace = faces[j];
		Plane splitterPlane = splitterFace.poly.getPlane();

		int frontCount = 0, backCount = 0, splitCount = 0;

		for (int i = 0; i < faces.size(); i++) {
			if (i == j) {
				continue;
			}

			
			SplittedPoly3D splitted = Geometry::split(faces[i].poly, splitterPlane);

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
			if (splitterFace.poly.getPlane().isAxial()) {
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

int BSP::addBSPFace(BSPFaceType type, const Face& face, int leafIdx)
{
	BSPFace bspFace = BSPFace(type, face, leafIdx);
	worldFaces.push_back(bspFace);

	return worldFaces.size() - 1;
}

int BSP::createLeaf()
{
	leaves.push_back(Leaf());
	return leaves.size() - 1;
}

int BSP::createPortal(const Poly3D& poly, int frontNodeIdx, int backNodeIdx)
{
	Portal portal;
	
	portal.valid = true;
	portal.poly = poly;
	portal.frontNodeIdx = frontNodeIdx;
	portal.backNodeIdx = backNodeIdx;
	portals.push_back(portal);

	return portals.size() - 1;
}

void BSP::compile(const FaceList& faces, const Vec3& start)
{
	nodes.clear();
	worldFaces.clear();

	BoundingBox worldBBox = makeBoundingBox(faces);
	FaceList worldBounds = makeWorldBounds(worldBBox, 100.0f);

	FaceList p2 = faces;

	for (const auto& p : worldBounds) {
		p2.push_back(p);
	}

	Context ctx;
	ctx.bounds = getPlanes(worldBBox);

	rootNodeIndex = createNode();
	ctx.nodeIdx = rootNodeIndex;

	build(p2, ctx);

	for (int portalIdx = 0; portalIdx < portals.size(); portalIdx++) {
		const Portal& portal = getPortal(portalIdx);

		if (!portal.valid || portal.frontNodeIdx == -1 || portal.backNodeIdx == -1) {
			continue;
		}

		const Node& node1 = getNode(portal.frontNodeIdx);
		const Node& node2 = getNode(portal.backNodeIdx);

		PlaneList bounds;

		if (node1.leafIdx != -1) {
			const Leaf& leaf = getLeaf(node1.leafIdx);
			bounds.insert(bounds.end(), leaf.bounds.begin(), leaf.bounds.end());
		}

		if (node2.leafIdx != -1) {
			const Leaf& leaf = getLeaf(node2.leafIdx);
			bounds.insert(bounds.end(), leaf.bounds.begin(), leaf.bounds.end());
		}

		Poly3D clippedPortalPoly = Geometry::clip(portal.poly, bounds);

		if (clippedPortalPoly.isValid()) {
			getPortal(portalIdx).poly = clippedPortalPoly;
		}
		else {
			getPortal(portalIdx).valid = false;
		}
	}

	floodFill(getLeafIdx(start));

	for (int leafIdx = 0; leafIdx < leaves.size(); leafIdx++) {
		const Leaf& leaf = getLeaf(leafIdx);

		if (leaf.valid) {
			continue;
		}

		for (int faceIdx : leaf.indices) {
			Vec3 n = worldFaces[faceIdx].face.poly.getPlane().normal;
			Vec3 p = worldFaces[faceIdx].face.poly.getCenter();
			
			p += n * 0.1f;
			int otherLeafIdx = getLeafIdx(p);

			if (otherLeafIdx == leafIdx) {
				continue;
			}

			const Leaf& otherLeaf = getLeaf(otherLeafIdx);

			if (!otherLeaf.valid) {
				continue;
			}
			std::cout << "Face returned from " << leafIdx << " to " << otherLeafIdx << "\n";
			getLeaf(otherLeafIdx).indices.push_back(faceIdx);
		}

	}

}

void BSP::build(const FaceList& faces, const Context& ctx)
{
	if (faces.size() == 0) {
		return;
	}

	int nodeIdx = ctx.nodeIdx;

	Splitter splitter = findSplitter(faces);

	if (splitter.valid) {

		int frontNodeIdx = createNode();
		int backNodeIdx = createNode();

		getNode(nodeIdx).frontIdx = frontNodeIdx;
		getNode(nodeIdx).backIdx = backNodeIdx;

		FaceList frontFaces, backFaces, coplanarFaces;
		Context frontCtx, backCtx;

		getNode(nodeIdx).leafIdx = -1;
		getNode(nodeIdx).plane = splitter.plane;
		
		// Разбиваем полигоны сплиттером
		for (const Face& face : faces) {

			
			SplittedPoly3D splittedPoly = Geometry::split(face.poly, splitter.plane);

			if (splittedPoly.result == SplitResult3D::Coplanar) {
				Face coplanarFace = Face(face, splittedPoly.coplanar);

				coplanarFaces.push_back(coplanarFace);
				frontFaces.push_back(coplanarFace);
			}
			else if (splittedPoly.result == SplitResult3D::Front) {
				frontFaces.push_back(Face(face, splittedPoly.front));
			}
			else if (splittedPoly.result == SplitResult3D::Back) {
				backFaces.push_back(Face(face, splittedPoly.back));
			}
			else if (splittedPoly.result == SplitResult3D::Intersected) {
				frontFaces.push_back(Face(face, splittedPoly.front));
				backFaces.push_back(Face(face, splittedPoly.back));
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
		for (int portalIdx : getNode(nodeIdx).portals) {
			SplittedPoly3D splittedPortalPoly = Geometry::split(getPortal(portalIdx).poly, splitter.plane);

			getPortal(portalIdx).valid = false;
			int otherIdx = getPortal(portalIdx).frontNodeIdx == nodeIdx ? getPortal(portalIdx).backNodeIdx : getPortal(portalIdx).frontNodeIdx;
			
			if (splittedPortalPoly.front.isValid()) {
				int portalIdx = createPortal(splittedPortalPoly.front, frontNodeIdx, otherIdx);
				getNode(frontNodeIdx).portals.push_back(portalIdx);
				getNode(otherIdx).portals.push_back(portalIdx);
			}

			if (splittedPortalPoly.back.isValid()) {
				int portalIdx = createPortal(splittedPortalPoly.back, backNodeIdx, otherIdx);
				getNode(backNodeIdx).portals.push_back(portalIdx);
				getNode(otherIdx).portals.push_back(portalIdx);
			}

			// В теории невозможно
			if (splittedPortalPoly.coplanar.isValid()) {
				int portalIdx = createPortal(splittedPortalPoly.coplanar, frontNodeIdx, otherIdx);
				getNode(frontNodeIdx).portals.push_back(portalIdx);
				getNode(otherIdx).portals.push_back(portalIdx);
			}

		}

		Poly3DList portalPolys = createPortalPolys(splitter.plane, ctx.bounds, coplanarFaces);

		// Новый портал сразу попадает в оба списка дочерних узлов для возможного будущего разделения
		for (const auto& portalPoly : portalPolys) {
			int portalIdx = createPortal(portalPoly, frontNodeIdx, backNodeIdx);
			getNode(frontNodeIdx).portals.push_back(portalIdx);
			getNode(backNodeIdx).portals.push_back(portalIdx);
		}

		// Формируем bounds для дочерних элементов
		frontCtx.nodeIdx = frontNodeIdx;
		frontCtx.bounds = ctx.bounds;
		frontCtx.bounds.push_back(nodes[nodeIdx].plane);

		backCtx.nodeIdx = backNodeIdx;
		backCtx.bounds = ctx.bounds;
		backCtx.bounds.push_back(splitter.plane.flip());

		build(frontFaces, frontCtx);
		build(backFaces, backCtx);
	}
	else {
		int leafIdx = createLeaf();

		getNode(nodeIdx).frontIdx = -1;
		getNode(nodeIdx).backIdx = -1;

		for (int i = 0; i < faces.size(); i++) {
			getLeaf(leafIdx).bounds.push_back(faces[i].poly.getPlane());
			getLeaf(leafIdx).indices.push_back(addBSPFace(BSPFaceType::Geometry, faces[i], leafIdx));
		}

		nodes[nodeIdx].leafIdx = leafIdx;
	}
}

void BSP::render(const Vec3& pos, World& world)
{
	/*
	if (renderOnlyCurrentLeaf) {
		int currentLeafIdx = getLeafIdx(pos);
		Leaf leaf2 = getLeaf(currentLeafIdx);

		if (leaf2.valid) {
			for (int index : leaf2.indices) {
				world.addGeometry(worldFaces[index].face);
			}
		}
	}
	else {
		
	}
	*/
	traverse(rootNodeIndex, pos, world);
}

FaceList BSP::renderPortals(const Vec3& pos)
{
	FaceList portalFaces;

	portalFaces.clear();

	int currentLeafIdx = getLeafIdx(pos);

	int i = 0;
	for (const auto& portal : portals) {
		if (portal.valid && getNode(portal.frontNodeIdx).leafIdx != -1 && getNode(portal.backNodeIdx).leafIdx != -1 && portal.visited) {
			int leafA = getNode(portal.frontNodeIdx).leafIdx;
			int leafB = getNode(portal.backNodeIdx).leafIdx;
			if (leafA == currentLeafIdx || leafB == currentLeafIdx) {
				if (leafA != currentLeafIdx) {
					std::swap(leafA, leafB);
				}
				portalFaces.push_back(Face(portal.poly, -1, Color::getDebugColor(leafA + leafB)));
			}
		}
		i++;
	}

	return portalFaces;
}

LabelList BSP::renderPortalLabels(const Vec3& pos)
{
	LabelList portalLabels;

	int currentLeafIdx = getLeafIdx(pos);

	int i = 0;
	for (const auto& portal : portals) {
		if (portal.valid && getNode(portal.frontNodeIdx).leafIdx != -1 && getNode(portal.backNodeIdx).leafIdx != -1 && portal.visited) {
			int leafA = getNode(portal.frontNodeIdx).leafIdx;
			int leafB = getNode(portal.backNodeIdx).leafIdx;
			if (leafA == currentLeafIdx || leafB == currentLeafIdx) {
				if (leafA != currentLeafIdx) {
					std::swap(leafA, leafB);
				}
				static char buf[256];
				sprintf_s(buf, "#%i %i <=> %i", i, leafA, leafB);
				portalLabels.push_back(Label(portal.poly.getCenter(), Color(255, 0, 0), buf));
			}
		}
		i++;
	}

	return portalLabels;

}

void BSP::traverse(int nodeIndex, const Vec3& pos, World& world)
{
	if (nodeIndex == -1) {
		return;
	}

	const Node& node = nodes[nodeIndex];

	if (node.leafIdx != -1)
	{
		Leaf leaf = leaves[node.leafIdx];

		if (leaf.valid) {
			for (int index : leaf.indices) {
				BSPFace bspFace = worldFaces[index];
				bspFace.face.debugColor = Color::getDebugColor(bspFace.leafIdx);
				world.addGeometry(bspFace.face);
			}
		}
	}
	else
	{
		Side side = Geometry::test(pos, node.plane);

		if (side == Side::Front) {
			traverse(node.backIdx, pos, world);
			traverse(node.frontIdx, pos, world);
		}
		else if (side == Side::Back) {
			traverse(node.frontIdx, pos, world);
			traverse(node.backIdx, pos, world);
		}
		else {
			traverse(node.frontIdx, pos, world);
			traverse(node.backIdx, pos, world);
		}
	}
}

int BSP::getLeafIdx(const Vec3& vec)
{
	int nodeId = rootNodeIndex;
	while (true) {
		const Node& node = nodes[nodeId];

		if (node.leafIdx != -1) {
			return node.leafIdx;
		}

		if (Geometry::test(vec, node.plane) == Side::Back) {
			nodeId = node.backIdx;
		}
		else {
			nodeId = node.frontIdx;
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

	poly.addVert(Vertex3D(center - axisU * size - axisV * size, Vec2(0, 0)));
	poly.addVert(Vertex3D(center + axisU * size - axisV * size, Vec2(1, 0)));
	poly.addVert(Vertex3D(center + axisU * size + axisV * size, Vec2(1, 1)));
	poly.addVert(Vertex3D(center - axisU * size + axisV * size, Vec2(0, 1)));

	return poly;
}

BoundingBox BSP::makeBoundingBox(const FaceList& faces)
{
	BoundingBox bbox;

	if (faces.size() == 0) {
		return bbox;
	}

	// FIX
	bbox.min = bbox.max = faces[0].poly.verts[0].position;

	for (const auto& face : faces) {

		for (const auto& vert : face.poly.verts) {
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

Poly3DList BSP::createPortalPolys(const Plane& plane, const PlaneList& bounds, const FaceList& coplanarFaces)
{
	Poly3DList portalPolys;
	Poly2DList portalPolys2D;

	// Создаем портал используя плоскость сплиттера
	Poly3D portalPoly = makePolygonOnPlane(plane);
	portalPoly = Geometry::clip(portalPoly, bounds);

	if (!portalPoly.isValid()) {
		return portalPolys;
	}

	PlaneBasis basis = Geometry::makePlaneBasis(plane);

	Poly2D portalPoly2D;
	
	for (int i = 0; i < portalPoly.getCount(); i++) {
		Vec2 proj = basis.project(portalPoly.verts[i].position);
		portalPoly2D.addVert({ proj, 1, 1, 1});
	}

	portalPolys2D.push_back(portalPoly2D);

	for (const auto& coplanarFace : coplanarFaces)
	{
		Poly2D coplanarPoly2D;

		for (int i = 0; i < coplanarFace.poly.getCount(); i++) {
			Vec2 proj = basis.project(coplanarFace.poly.verts[i].position);
			coplanarPoly2D.addVert({ proj, 1, 1, 1 });
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
		for (int i = 0; i < portalPoly2D.getCount(); i++) {
			portalPoly.addVert(Vertex3D(basis.unproject(portalPoly2D.verts[i].pos), Vec2(1, 1)));
		}

		portalPolys.push_back(portalPoly);
	}

	return portalPolys;
}

void BSP::floodFill(int leafIdx)
{
	for (int i = 0; i < portals.size(); i++) {
		if (getPortal(i).valid && !getPortal(i).visited && getNode(getPortal(i).frontNodeIdx).leafIdx != -1 && getNode(getPortal(i).backNodeIdx).leafIdx != -1) {
			int currentLeafIdx = getNode(getPortal(i).frontNodeIdx).leafIdx;
			int nextLeafIdx = getNode(getPortal(i).backNodeIdx).leafIdx;

			if (currentLeafIdx == leafIdx || nextLeafIdx == leafIdx) {
				
				if (currentLeafIdx != leafIdx) {
					std::swap(currentLeafIdx, nextLeafIdx);
				}
				leaves[currentLeafIdx].valid = true;
				leaves[nextLeafIdx].valid = true;
				getPortal(i).visited = true;
				floodFill(nextLeafIdx);
			}
		}
	}
}

FaceList BSP::makeWorldBounds(const BoundingBox& src, float padding)
{
	FaceList polys;
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
		polys.push_back(Face(Poly3D({ {vec1, {0, 0}}, {vec2, {0, 1}}, {vec3, {1, 1}}, {vec4, {1, 0}} }), -1));
	};

	addPoly(v000, v001, v011, v010);
	addPoly(v100, v110, v111, v101);
	addPoly(v000, v100, v101, v001);
	addPoly(v010, v011, v111, v110);
	addPoly(v000, v010, v110, v100);
	addPoly(v001, v101, v111, v011);

	return polys;
}