#pragma once

#include <vector>

#include "Vector.h"
#include "Triangle.h"
#include "Math.h"
#include "World.h"
#include "Camera.h"

using IndicesList = std::vector<int>;

struct Node
{
	Plane plane;
	IndicesList coplanar;

	Polygon3D portal;

	int leafIndex;
	int frontIndex;
	int backIndex;
};

struct Leaf
{
	IndicesList indices;
};

using LeafList = std::vector<Leaf>;

enum Side
{
	Front,
	Back,
	Coplane
};

struct BoundingBox
{
	Vec3 min, max;
};

struct SplittedPolygon
{
	bool coplanar = false;
	Polygon3D front;
	Polygon3D back;
};

struct SplittedTriangle
{
	bool coplanar = false;
	Triangle3DList front;
	Triangle3DList back;
};

using NodeList = std::vector<Node>;

class BSP
{
public:
	
	void compile(const Triangle3DList& tris);
	void render(const Camera& cam, World& world);
	int leaf(const Camera& cam);

	Triangle3DList portalTris;

private:
	int rootNodeIndex = 0;
	NodeList nodes;
	LeafList leaves;
	Triangle3DList list;

	int addToList(const Triangle3& tri);
	int addLeaf(const Leaf& leaf);
	Triangle3 getFromList(int index);

	int findSplitter(const Triangle3DList& tris);
	int build(const Triangle3DList& tris, const PlaneList& bounds);
	void traverse(int nodeIndex, const Camera& cam, World& world);
	SplittedTriangle splitTriangle(const Triangle3& tri, const Plane& plane);
	Side test(const Plane& plane, const Vec3& vec);
	Vertex3D intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane);
	SplittedPolygon splitAgainstPlane(const Polygon3D& input, const Plane& plane);

	Polygon3D makePolygonOnPlane(const Plane& plane);
	Polygon3D clipPolygon(const Polygon3D& poly, const PlaneList& bounds);

	Triangle3DList triangulate(const Polygon3D& poly, size_t texId, const Color& fill);
	BoundingBox makeBoundingBox(const Triangle3DList& tris);
	PlaneList getPlanes(const BoundingBox& bbox); 
};

