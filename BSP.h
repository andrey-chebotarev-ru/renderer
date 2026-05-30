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
	
	IndicesList portals;

	int leafIdx;
	int frontIdx;
	int backIdx;
};

using NodeList = std::vector<Node>;

struct Leaf
{
	bool valid = false;
	IndicesList indices;
	PlaneList bounds;
};

using LeafList = std::vector<Leaf>;

struct Portal
{
	Poly3D poly;
	bool valid;
	int frontNodeIdx;
	int backNodeIdx;
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

enum class BSPFaceType
{
	Geometry,
	Bounds,
	Portal
};

struct BSPFace
{
	BSPFaceType type;
	Face face;
	int leafIdx;

	BSPFace(BSPFaceType type, const Face& face, int leafIdx)
		: type(type), face(face), leafIdx(leafIdx) {
	}
};

using BSPFaceList = std::vector<BSPFace>;

class BSP
{
public:
	
	void compile(const FaceList& polys, const Vec3& start);
	void render(const Vec3& pos, World& world);
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

	void floodFill(int leafId);

	int createPortal(const Poly3D& poly, int frontIndex, int backIndex);
	Poly3DList createPortalPolys(const Plane& plane, const PlaneList& bounds, const FaceList& coplanarFaces);


	int addBSPFace(BSPFaceType type, const Face& face, int leafIdx);
	int createLeaf();

	Poly3D getFromList(int index);

	Splitter findSplitter(const FaceList& polys);

	void build(const FaceList& faces, const Context& ctx);
	void traverse(int nodeIndex, const Vec3& pos, World& world);


	Poly3D makePolygonOnPlane(const Plane& plane);

	BoundingBox makeBoundingBox(const FaceList& polys);
	PlaneList getPlanes(const BoundingBox& bbox);
	FaceList makeWorldBounds(const BoundingBox& bbox, float padding);
};

