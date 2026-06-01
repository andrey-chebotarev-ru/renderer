#pragma once

#include <vector>
#include <functional>
#include <string>

#include "Vector.h"
#include "Triangle.h"
#include "Math.h"
#include "World.h"
#include "Camera.h"
#include "Geometry.h"

using IndicesList = std::vector<int>;

struct Node
{
	Plane plane;
	PlaneList bounds;
	FaceList coplanarFaces;

	int leafIdx;
	int frontIdx;
	int backIdx;
};

using NodeList = std::vector<Node>;

struct Leaf
{
	bool valid = false;
	bool outside = false;

	IndicesList faceIdxs;
	IndicesList portalIdxs;

	PlaneList bounds;
};

using LeafList = std::vector<Leaf>;

struct Portal
{
	Poly3D poly;
	
	int nodeIdx = -1;
	
	int frontNodeIdx = -1;
	int backNodeIdx = -1;

	int frontLeafIdx = -1;
	int backLeafIdx = -1;
	
	bool visited = false;
};

using PortalList = std::vector<Portal>;

struct BoundingBox
{
	Vec3 min, max;
};

struct Context
{
	int nodeIdx;
	PlaneList bounds;
};

struct Splitter
{
	bool valid;
	Plane plane;
	int idx;
};

struct BSPFace
{
	Face face;
	int leafIdx;

	BSPFace(const Face& face, int leafIdx)
		: face(face), leafIdx(leafIdx) {
	}
};

using BSPFaceList = std::vector<BSPFace>;

enum class PortalSide
{
	A,
	B
};

class BSP
{
public:

	bool compile(const FaceList& polys, const Vec3& start);
	void render(const Vec3& pos, World& world, bool includePortals = false);
	FaceList renderPortals(const Vec3& pos);
	LabelList renderPortalLabels(const Vec3& pos);

	int getLeafIdx(const Vec3& pos);


	const Leaf& getLeaf(int index) const {
		return leaves[index];
	}

	Leaf& getLeaf(int index) {
		return leaves[index];
	}

private:
	int rootNodeIndex = 0;
	NodeList nodes;
	LeafList leaves;
	BSPFaceList worldFaces;
	PortalList portals;

	int createNode();

	Node& getNode(int index) {
		return nodes[index];
	}

	const Node& getNode(int index) const {
		return nodes[index];
	}

	Portal& getPortal(int index) {
		return portals[index];
	}

	const Portal& getPortal(int index) const {
		return portals[index];
	}

	bool floodFill(int leafId);

	int addPortal(const Portal& portal);
	Portal createPortal(int nodeIdx);
	PortalList carvePortal(const Portal& portal);

	int addBSPFace(const Face& face, int leafIdx);
	int createLeaf();

	Poly3D getFromList(int index);

	Splitter findSplitter(const FaceList& polys);

	void build(const FaceList& faces, const Context& ctx);
	void traverse(int nodeIndex, const Vec3& pos, World& world);


	Poly3D makePolygonOnPlane(const Plane& plane);

	BoundingBox makeBoundingBox(const FaceList& polys);
	PlaneList getPlanes(const BoundingBox& bbox);
	FaceList makeWorldBounds(const BoundingBox& bbox, float padding);

	void traceFace(const BSPFace& face, int nodeIdx);

	void buildPortalsFromNode(int nodeIdx);
	void pushPortal(Portal portal, int nodeAIdx, int nodeBIdx);
	void refinePortalSide(const Portal& portal, int splitNodeIdx, int otherNodeIdx, PortalSide side);
	bool clipPortal(Portal& portal);
	void restoreBorderFaces();
};