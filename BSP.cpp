#include "BSP.h"

#include <iostream>
#include <cstdlib>

#include <SDL3/SDL.h>

Leaf BSP::getLeaf(const Vec3& pos) const
{
	return getLeaf(getLeafIdx(pos));
}

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

		if (!splitterFace.canSplit) {
			continue;
		}

		int coplanarCount = 0, frontCount = 0, backCount = 0, splitCount = 0;

		for (int i = 0; i < faces.size(); i++) {
			if (i == j) {
				continue;
			}


			SplittedPoly3D splitted = Geometry::split(faces[i].poly, splitterPlane);

			if (splitted.result == SplitResult3D::Coplanar) {
				coplanarCount++;
			}
			else if (splitted.result == SplitResult3D::Front) {
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

		if (backCount > 0) {
			int score = splitCount * 16 + abs(backCount - frontCount) - coplanarCount * 2;
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
	tree.nodes.push_back(Node());
	return tree.nodes.size() - 1;
}

int BSP::addBSPFace(const Face& face, int leafIdx)
{
	BSPFace bspFace = BSPFace(face, leafIdx);
	tree.faces.push_back(bspFace);

	return tree.faces.size() - 1;
}

int BSP::createLeaf()
{
	tree.leaves.push_back(Leaf());
	return tree.leaves.size() - 1;
}

int BSP::addPortal(const Portal& portal)
{
	tree.portals.push_back(portal);

	return tree.portals.size() - 1;
}

bool BSP::compile(const FaceList& faces, const Vec3& start)
{
	tree.nodes.clear();
	tree.faces.clear();

	AABB3D worldAABB = makeAABB(faces);
	FaceList worldBounds = makeWorldBounds(worldAABB, 10.0f);

	FaceList p2 = faces;

	for (const auto& p : worldBounds) {
		p2.push_back(p);
	}

	Context ctx;
	ctx.bounds = getPlanes(worldAABB);

	tree.rootNodeIdx = createNode();
	ctx.nodeIdx = tree.rootNodeIdx;

	tree.portals.clear();

	build(faces, ctx);

	buildPortalsFromNode(tree.rootNodeIdx);

	bool res = floodFill(getLeafIdx(start));

	//restoreBorderFaces();
	return res;
}

void BSP::pushFace(const BSPFace& face, int nodeIdx)
{
	if (getNode(nodeIdx).leafIdx == -1) {
		SplitResult3D classification = Geometry::classify(Poly3D::offset(face.face.poly, 0.01f), getNode(nodeIdx).plane);



		if (classification == SplitResult3D::Coplanar) {

			if (Vec3::dot(face.face.poly.getPlane().normal, getNode(nodeIdx).plane.normal) > 0) {
				pushFace(face, getNode(nodeIdx).frontIdx);
			}
			else {
				pushFace(face, getNode(nodeIdx).backIdx);
			}
		}
		else if (classification == SplitResult3D::Front) {
			pushFace(face, getNode(nodeIdx).frontIdx);
		}
		else if (classification == SplitResult3D::Back) {
			pushFace(face, getNode(nodeIdx).backIdx);
		}
		else if (classification == SplitResult3D::Intersected) {
			SplittedPoly3D splittedPoly = Geometry::split(face.face.poly, getNode(nodeIdx).plane);
			pushFace(BSPFace(Face(face.face, splittedPoly.front), getNode(nodeIdx).frontIdx), getNode(nodeIdx).frontIdx);
			pushFace(BSPFace(Face(face.face, splittedPoly.back), getNode(nodeIdx).backIdx), getNode(nodeIdx).backIdx);
		}
	}
	else {
		if (getLeaf(getNode(nodeIdx).leafIdx).valid) {
			getLeaf(getNode(nodeIdx).leafIdx).faceIdxs.push_back(addBSPFace(face.face, getNode(nodeIdx).leafIdx));
		}
	}

}

void BSP::buildPortalsFromNode(int nodeIdx)
{
	if (getNode(nodeIdx).leafIdx != -1) {
		return;
	}

	Portal portal = createPortal(nodeIdx);

	pushPortal(portal, getNode(nodeIdx).frontIdx, getNode(nodeIdx).backIdx);

	buildPortalsFromNode(getNode(nodeIdx).frontIdx);
	buildPortalsFromNode(getNode(nodeIdx).backIdx);
}

void BSP::pushPortal(Portal portal, int nodeAIdx, int nodeBIdx)
{
	bool aIsLeaf = (getNode(nodeAIdx).leafIdx != -1);
	bool bIsLeaf = (getNode(nodeBIdx).leafIdx != -1);

	if (aIsLeaf && bIsLeaf) {
		int leafA = getNode(nodeAIdx).leafIdx;
		int leafB = getNode(nodeBIdx).leafIdx;

		if (leafA != leafB) {
			portal.frontNodeIdx = nodeAIdx;
			portal.backNodeIdx = nodeBIdx;
			portal.frontLeafIdx = leafA;
			portal.backLeafIdx = leafB;


			if (clipPortal(portal)) {
				PortalList finalPortals = carvePortal(portal);

				for (const auto& finalPortal : finalPortals) {
					int portalIdx = addPortal(finalPortal);
					getLeaf(leafA).portalIdxs.push_back(portalIdx);
					getLeaf(leafB).portalIdxs.push_back(portalIdx);
				}
			}
		}

		return;
	}

	if (!aIsLeaf) {
		refinePortalSide(portal, nodeAIdx, nodeBIdx, PortalSide::A);
		return;
	}

	if (!bIsLeaf) {
		refinePortalSide(portal, nodeBIdx, nodeAIdx, PortalSide::B);
		return;
	}
}

void BSP::refinePortalSide(const Portal& portal, int splitNodeIdx, int otherNodeIdx, PortalSide side)
{
	SplitResult3D cls = Geometry::classify(portal.poly, getNode(splitNodeIdx).plane);

	if (cls == SplitResult3D::Front) {

		if (side == PortalSide::A) {
			pushPortal(portal, getNode(splitNodeIdx).frontIdx, otherNodeIdx);
		}
		else {
			pushPortal(portal, otherNodeIdx, getNode(splitNodeIdx).frontIdx);
		}

		return;
	}

	if (cls == SplitResult3D::Back) {

		if (side == PortalSide::A) {
			pushPortal(portal, getNode(splitNodeIdx).backIdx, otherNodeIdx);
		}
		else {
			pushPortal(portal, otherNodeIdx, getNode(splitNodeIdx).backIdx);
		}

		return;
	}

	if (cls == SplitResult3D::Intersected) {
		SplittedPoly3D splittedPortalPoly = Geometry::split(portal.poly, getNode(splitNodeIdx).plane);
		Portal frontPortal, backPortal;

		frontPortal = backPortal = portal;

		frontPortal.poly = splittedPortalPoly.front;
		backPortal.poly = splittedPortalPoly.back;

		if (side == PortalSide::A) {
			pushPortal(frontPortal, getNode(splitNodeIdx).frontIdx, otherNodeIdx);
			pushPortal(backPortal, getNode(splitNodeIdx).backIdx, otherNodeIdx);
		}
		else {
			pushPortal(frontPortal, otherNodeIdx, getNode(splitNodeIdx).frontIdx);
			pushPortal(backPortal, otherNodeIdx, getNode(splitNodeIdx).backIdx);
		}

		return;
	}

	// Невозможно
	if (cls == SplitResult3D::Coplanar) {
		//pushPortal(portal, getNode(splitNodeIdx).frontIdx, otherNodeIdx);
		//pushPortal(portal, getNode(splitNodeIdx).backIdx, otherNodeIdx);

		return;
	}
}

/*
* Обрезание портала плоскостями граничащих с ним листьев
*/
bool BSP::clipPortal(Portal& portal)
{
	const Leaf& leafA = getLeaf(portal.frontLeafIdx);
	const Leaf& leafB = getLeaf(portal.backLeafIdx);

	PlaneList bounds;

	bounds.insert(bounds.end(), leafA.bounds.begin(), leafA.bounds.end());
	bounds.insert(bounds.end(), leafB.bounds.begin(), leafB.bounds.end());
	
	portal.poly = Geometry::clip(portal.poly, bounds);

	return portal.poly.isValid();
}

void BSP::restoreBorderFaces()
{
	for (int leafIdx = 0; leafIdx < tree.leaves.size(); leafIdx++) {
		const Leaf& leaf = getLeaf(leafIdx);

		if (leaf.valid) {
			continue;
		}

		for (int faceIdx : leaf.faceIdxs) {
			Vec3 n = tree.faces[faceIdx].face.poly.getPlane().normal;
			Vec3 p = tree.faces[faceIdx].face.poly.getCenter();

			p += n * 0.1f;
			int otherLeafIdx = getLeafIdx(p);

			if (otherLeafIdx == leafIdx) {
				continue;
			}

			const Leaf& otherLeaf = getLeaf(otherLeafIdx);

			if (!otherLeaf.valid) {
				continue;
			}

			auto face = tree.faces[faceIdx];
			face.face.poly = Poly3D::offset(face.face.poly, 0.01f);

			pushFace(face, tree.rootNodeIdx);

			std::cout << "Face returned from " << leafIdx << " to " << otherLeafIdx << "\n";
		}
	}
}

void BSP::build(const FaceList& faces, const Context& ctx)
{
	if (faces.size() == 0) {
		return;
	}

	if (ctx.depth > 100) {
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


		int idx = 0;
		// Разбиваем полигоны сплиттером
		for (const Face& face : faces) {

			SplittedPoly3D splittedPoly = Geometry::split(face.poly, splitter.plane);

			if (splittedPoly.result == SplitResult3D::Coplanar) {
				Face coplanarFace = Face(face, splittedPoly.coplanar);

				if (1) {
					coplanarFace.canSplit = false;
				}

				coplanarFaces.push_back(coplanarFace);
				
				if (Vec3::dot(coplanarFace.poly.getPlane().normal, splitter.plane.normal) < 0.0f) {
					backFaces.push_back(coplanarFace);
				}
				else {
					frontFaces.push_back(coplanarFace);
				}

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

			idx++;
		}

		getNode(nodeIdx).bounds = ctx.bounds;
		getNode(nodeIdx).coplanarFaces = coplanarFaces;

		// Формируем bounds для дочерних элементов
		frontCtx.depth = ctx.depth + 1;
		frontCtx.nodeIdx = frontNodeIdx;
		frontCtx.bounds = ctx.bounds;
		frontCtx.bounds.push_back(splitter.plane);

		backCtx.depth = ctx.depth + 1;
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

		for (const auto& face : faces) {
			if (face.type == FaceType::Bounds) {
				getLeaf(leafIdx).outside = true;

			}
			getLeaf(leafIdx).bounds.push_back(face.poly.getPlane());
			getLeaf(leafIdx).faceIdxs.push_back(addBSPFace(face, leafIdx));
		}

		tree.nodes[nodeIdx].leafIdx = leafIdx;
	}
}

void BSP::render(const Vec3& pos, World& world, bool includePortals)
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


	traverse(tree.rootNodeIdx, pos, world);

	if (includePortals) {
		int currentLeafIdx = getLeafIdx(pos);
		const Leaf& leaf = getLeaf(currentLeafIdx);
		for (int portalIdx : leaf.portalIdxs) {
			const Portal& portal = getPortal(portalIdx);
			if (portal.visited) {
				int otherLeafIdx = portal.frontLeafIdx == currentLeafIdx ? portal.backLeafIdx : portal.frontLeafIdx;
				world.addGeometry(Face(portal.poly, -1, FaceType::Debug, Color::getDebugColor(currentLeafIdx + otherLeafIdx)));
			}
		}
	}
}

FaceList BSP::renderPortals(const Vec3& pos)
{
	FaceList portalFaces;

	portalFaces.clear();

	int currentLeafIdx = getLeafIdx(pos);
	const Leaf& leaf = getLeaf(currentLeafIdx);

	for (int portalIdx : leaf.portalIdxs) {
		const Portal& portal = getPortal(portalIdx);
		if (portal.visited) {
			int otherLeafIdx = portal.frontLeafIdx == currentLeafIdx ? portal.backLeafIdx : portal.frontLeafIdx;
			//portalFaces.push_back(Face(portal.poly, -1, FaceType::Debug, Color::getDebugColor(currentLeafIdx + otherLeafIdx)));
		}
	}

	return portalFaces;
}

LabelList BSP::renderPortalLabels(const Vec3& pos)
{
	LabelList portalLabels;
	static char buf[256];

	int currentLeafIdx = getLeafIdx(pos);
	const Leaf& leaf = getLeaf(currentLeafIdx);

	for (int portalIdx : leaf.portalIdxs) {
		const Portal& portal = getPortal(portalIdx);
		if (portal.visited) {
			int otherLeafIdx = portal.frontLeafIdx == currentLeafIdx ? portal.backLeafIdx : portal.frontLeafIdx;
			sprintf_s(buf, "#%i %i <=> %i", portalIdx, currentLeafIdx, otherLeafIdx);
			portalLabels.push_back(Label(portal.poly.getCenter(), Color(255, 0, 0), buf));
		}
	}

	return portalLabels;
}

void BSP::traverse(int nodeIndex, const Vec3& pos, World& world)
{
	if (nodeIndex == -1) {
		return;
	}

	const Node& node = getNode(nodeIndex);

	if (node.leafIdx != -1)
	{
		const Leaf& leaf = getLeaf(node.leafIdx);

		if (leaf.valid && leaf.visible) {
			for (int faceIdx : leaf.faceIdxs) {
				BSPFace bspFace = tree.faces[faceIdx];
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

int BSP::getLeafIdx(const Vec3& vec) const
{
	int nodeIdx = tree.rootNodeIdx;

	while (true) {
		const Node& node = getNode(nodeIdx);

		if (node.leafIdx != -1) {
			return node.leafIdx;
		}

		if (Geometry::test(vec, node.plane) == Side::Back) {
			nodeIdx = node.backIdx;
		}
		else {
			nodeIdx = node.frontIdx;
		}
	}

	return 0;
}

AABB3D BSP::makeAABB(const FaceList& faces)
{
	AABB3D aabb;

	if (faces.size() == 0) {
		return aabb;
	}

	// FIX
	aabb.min = aabb.max = faces[0].poly.verts[0].pos;

	for (const auto& face : faces) {

		for (const auto& vert : face.poly.verts) {
			aabb.min.x = std::min(aabb.min.x, vert.pos.x);
			aabb.min.y = std::min(aabb.min.y, vert.pos.y);
			aabb.min.z = std::min(aabb.min.z, vert.pos.z);

			aabb.max.x = std::max(aabb.max.x, vert.pos.x);
			aabb.max.y = std::max(aabb.max.y, vert.pos.y);
			aabb.max.z = std::max(aabb.max.z, vert.pos.z);
		}
	}

	return aabb;
}

PlaneList BSP::getPlanes(const AABB3D& aabb)
{
	PlaneList planes;

	planes.push_back(Plane(Vec3(-1, 0, 0), -aabb.max.x));
	planes.push_back(Plane(Vec3(1, 0, 0), aabb.min.x));

	planes.push_back(Plane(Vec3(0, -1, 0), -aabb.max.y));
	planes.push_back(Plane(Vec3(0, 1, 0), aabb.min.y));

	planes.push_back(Plane(Vec3(0, 0, -1), -aabb.max.z));
	planes.push_back(Plane(Vec3(0, 0, 1), aabb.min.z));

	return planes;
}

/*
* Создание портала
* Берем огромный полигон на плоскости ноды (сплиттера) и обрезаем всеми родительскими плоскостями
*/
Portal BSP::createPortal(int nodeIdx)
{
	Portal portal;

	const Node& node = getNode(nodeIdx);

	portal.nodeIdx = nodeIdx;
	portal.poly = Geometry::makePolygonOnPlane(node.plane, 100000.0f);
	portal.poly = Geometry::clip(portal.poly, node.bounds);

	return portal;
}

/*
* Вырезание из портала solid-геометрии
* Переводим портал и копланарную геометрию ноды в 2D-вид и последовательно вычитаем из портала каждый копланарный полигон
* Результат - множество порталов которые уцелели
*/
PortalList BSP::carvePortal(const Portal& in)
{
	PortalList out;

	Poly3DList pieces;
	pieces.push_back(in.poly);

	for (const auto& face : getNode(in.nodeIdx).coplanarFaces)
	{
		Poly3DList next;

		for (const auto& piece : pieces) {
			Poly3DList polys = Geometry::subtract(piece, face.poly, in.poly.getPlane(), 1.0f);
			next.insert(next.end(), polys.begin(), polys.end());
		}

		pieces = std::move(next);

		if (pieces.empty()) {
			break;
		}
	}

	for (const auto& poly : pieces) {
		Portal p = in;
		p.poly = poly;

		out.push_back(p);
	}

	return out;
}

/*
bool BSP::floodFill(int leafIdx)
{
	const Leaf& leaf = getLeaf(leafIdx);
	bool leakDetected = leaf.outside;
	
	getLeaf(leafIdx).valid = true;

	for (int portalIdx : leaf.portalIdxs) {
		const Portal& portal = getPortal(portalIdx);
		if (!portal.visited) {
			int otherLeafIdx = (portal.frontLeafIdx == leafIdx ? portal.backLeafIdx : portal.frontLeafIdx);
			getLeaf(otherLeafIdx).valid = true;
			getPortal(portalIdx).visited = true;
			getPortal(portalIdx).visited = true;
			if (floodFill(otherLeafIdx)) {
				leakDetected = true;
			}
		}
	}

	return leakDetected;
}
*/


bool BSP::floodFill(int startLeafIdx)
{
	bool leakDetected = false;
	IndicesList stack;
	
	getLeaf(startLeafIdx).visited = true;
	getLeaf(startLeafIdx).valid = true;
	stack.push_back(startLeafIdx);

	while (!stack.empty()) {
		int leafIdx = stack.back();
		stack.pop_back();

		for (int portalIdx : getLeaf(leafIdx).portalIdxs) {
			const Portal& p = getPortal(portalIdx);

			getPortal(portalIdx).visited = true;

			int nextLeafIdx = p.frontLeafIdx == leafIdx ? p.backLeafIdx : (p.backLeafIdx == leafIdx ? p.frontLeafIdx : -1);

			if (nextLeafIdx == -1) {
				continue;
			}

			if (getLeaf(nextLeafIdx).visited) {
				continue;
			}

			if (getLeaf(nextLeafIdx).outside) {
				leakDetected = true;
			}

			getLeaf(nextLeafIdx).visited = true;
			getLeaf(nextLeafIdx).valid = true;
			stack.push_back(nextLeafIdx);
		}

	}

	return leakDetected;
}

FaceList BSP::makeWorldBounds(const AABB3D& src, float padding)
{
	FaceList polys;
	AABB3D aabb = src;

	aabb.min -= padding;
	aabb.max += padding;

	Vec3 v000 = Vec3(aabb.min.x, aabb.min.y, aabb.min.z);
	Vec3 v001 = Vec3(aabb.min.x, aabb.min.y, aabb.max.z);
	Vec3 v010 = Vec3(aabb.min.x, aabb.max.y, aabb.min.z);
	Vec3 v011 = Vec3(aabb.min.x, aabb.max.y, aabb.max.z);

	Vec3 v100 = Vec3(aabb.max.x, aabb.min.y, aabb.min.z);
	Vec3 v101 = Vec3(aabb.max.x, aabb.min.y, aabb.max.z);
	Vec3 v110 = Vec3(aabb.max.x, aabb.max.y, aabb.min.z);
	Vec3 v111 = Vec3(aabb.max.x, aabb.max.y, aabb.max.z);

	auto addPoly = [&](const Vec3& vec1, const Vec3& vec2, const Vec3& vec3, const Vec3& vec4) {
		polys.push_back(Face(Poly3D({ {vec4, {0, 0}}, {vec3, {0, 1}}, {vec2, {1, 1}}, {vec1, {1, 0}} }), 1, FaceType::Bounds));
		};

	addPoly(v000, v001, v011, v010);
	addPoly(v100, v110, v111, v101);
	addPoly(v000, v100, v101, v001);
	addPoly(v010, v011, v111, v110);
	addPoly(v000, v010, v110, v100);
	addPoly(v001, v101, v111, v011);

	return polys;
}

PortalList BSP::getPortalsInLeaf(int leafIdx) const
{
	PortalList portals;

	const Leaf& leaf = getLeaf(leafIdx);

	for (int portalIdx : leaf.portalIdxs) {
		portals.push_back(getPortal(portalIdx));
	}

	return portals;
}