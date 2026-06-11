#pragma once

#include <vector>
#include <functional>
#include <string>

#include "Vector.h"
#include "Triangle.h"
#include "Math.h"
#include "RenderFaces.h"
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
	bool visible = false;
	bool visited = false;

	IndicesList faceIdxs;
	IndicesList portalIdxs;

	PlaneList bounds;

	std::vector<AABB2D> vistedAABBs;
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

struct Context
{
	int nodeIdx;
	PlaneList bounds;
	size_t depth = 0;
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

	BSPFace() {}

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

struct Tree
{
	NodeList nodes;
	LeafList leaves;
	BSPFaceList faces;
	PortalList portals;
	size_t rootNodeIdx;
};

class BSP
{
public:

	BSP(Tree& tree) :
		tree(tree) {
	}

	bool compile(FaceList& polys, const Vec3List& playerStart);
	void render(const Vec3& pos, RenderFaces& renderFaces, bool includePortals = false);
	FaceList renderPortals(const Vec3& pos);
	LabelList renderPortalLabels(const Vec3& pos);

	PortalList getPortalsInLeaf(int leafIdx) const;

	int getLeafIdx(const Vec3& pos) const;

	Leaf getLeaf(const Vec3& pos) const;

	const Leaf& getLeaf(int index) const {
		return tree.leaves[index];
	}

	Leaf& getLeaf(int index) {
		return tree.leaves[index];
	}

	Portal& getPortal(int index) {
		return tree.portals[index];
	}

	const Portal& getPortal(int index) const {
		return tree.portals[index];
	}

	int getLeafCount() const {
		return tree.leaves.size();
	}

	int getPortalCount() const {
		return tree.portals.size();
	}

private:
	Tree& tree;

	int createNode();

	Node& getNode(int index) {
		return tree.nodes[index];
	}

	const Node& getNode(int index) const {
		return tree.nodes[index];
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
	void traverse(int nodeIndex, const Vec3& pos, RenderFaces& renderFaces);

	AABB3D makeAABB(const FaceList& polys);
	PlaneList getPlanes(const AABB3D& aabb);
	FaceList makeWorldBounds(const AABB3D& aabb, float padding);

	void pushFace(const BSPFace& face, int nodeIdx);

	void buildPortalsFromNode(int nodeIdx);
	void pushPortal(Portal portal, int nodeAIdx, int nodeBIdx);
	void refinePortalSide(const Portal& portal, int splitNodeIdx, int otherNodeIdx, PortalSide side);
	bool clipPortal(Portal& portal);
	void restoreBorderFaces();
};