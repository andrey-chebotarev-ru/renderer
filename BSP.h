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

	int leafIndex;
	int frontIndex;
	int backIndex;
};

struct Leaf
{
	bool valid = false;
	IndicesList indices;
};

using LeafList = std::vector<Leaf>;

struct Portal
{
	Poly3D poly;
	bool valid;
	int frontIndex;
	int backIndex;
	bool visited = false;
};

enum class WorldPolyType
{
	WorldPolyGeometry,
	WorldPolyBounds,
	WorldPolyPortal
};

struct WorldPoly
{
	WorldPolyType type;
	Poly3D poly;

	WorldPoly() {};
	WorldPoly(const Poly3D& poly, WorldPolyType type = WorldPolyType::WorldPolyGeometry)
		: poly(poly), type(type) {
	}
};

using WorldPolyList = std::vector<WorldPoly>;

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

using NodeList = std::vector<Node>;

class BSP
{
public:
	
	void compile(const Poly3DList& polys, const Vec3& start);
	void render(const Vec3& pos, World& world, bool populateLabels, bool renderOnlyCurrentLeaf);
	int getLeafIdx(const Vec3& vec);

	Poly3DList portalPolys;
	LabelList portalLabels;
	LabelList visibleLabels;

	const Leaf& leaf(int index) const {
		return leaves[index];
	}

private:
	int rootNodeIndex = 0;
	NodeList nodes;
	LeafList leaves;
	WorldPolyList worldPolys;
	PortalList portals;

	int createNode();

	Node& node(int index) {
		return nodes[index];
	}
	
	const Node& node(int index) const {
		return nodes[index];
	}

	Portal& portal(int index) {
		return portals[index];
	}

	const Portal& portal(int index) const {
		return portals[index];
	}

	void floodFill(int leafId);
	void traceNodePoly(const WorldPoly& nodePoly, int nodeIdx);

	int createPortal(const Poly3D& poly, int frontIndex, int backIndex);
	Poly3DList createPortalPolys(const Plane& plane, const PlaneList& bounds, const Poly3DList& coplanarPolys);


	int addWorldPoly(const Poly3D& poly, int frontLeafIdx, int backLeafIdx);
	int addLeaf(const Leaf& leaf);

	Poly3D getFromList(int index);

	Splitter findSplitter(const Poly3DList& polys);

	void build(const Poly3DList& polys, const Context& ctx);
	void traverse(int nodeIndex, const Vec3& pos, World& world);

	Poly3D makePolygonOnPlane(const Plane& plane);
	Poly3D clipPolygon(const Poly3D& poly, const PlaneList& bounds);

	BoundingBox makeBoundingBox(const Poly3DList& polys);
	PlaneList getPlanes(const BoundingBox& bbox);
	WorldPolyList makeWorldBounds(const BoundingBox& bbox, float padding);
};

