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
	IndicesList coplanar;

	IndicesList portals;

	int leafIndex;
	int frontIndex;
	int backIndex;
};

struct Leaf
{
	bool visited = false;
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



using PortalList = std::vector<Portal>;

struct BoundingBox
{
	Vec3 min, max;
};



using NodeList = std::vector<Node>;

class BSP
{
public:
	
	void compile(const Poly3DList& polys, const Vec3& start);
	void render(const Vec3& pos, World& world, bool populateLabels);
	int leaf(const Vec3& vec);

	Poly3DList portalPolys;
	LabelList portalLabels;
	LabelList visibleLabels;

private:
	int rootNodeIndex = 0;
	NodeList nodes;
	LeafList leaves;
	Poly3DList list;
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
		
	int createPortal(const Poly3D& poly, int frontIndex, int backIndex);
	Poly3DList createPortalPolys(const Plane& plane, const PlaneList& bounds, const IndicesList coplanarIdxs);


	int addToList(const Poly3D& poly);
	int addLeaf(const Leaf& leaf);

	Poly3D getFromList(int index);

	int findSplitter(const Poly3DList& polys);
	void build(int parentNodeIdx, const Poly3DList& polys, const PlaneList& bounds);
	void traverse(int nodeIndex, const Vec3& pos, World& world);
	

	Poly3D makePolygonOnPlane(const Plane& plane);
	Poly3D clipPolygon(const Poly3D& poly, const PlaneList& bounds);

	BoundingBox makeBoundingBox(const Poly3DList& polys);
	PlaneList getPlanes(const BoundingBox& bbox);
};

