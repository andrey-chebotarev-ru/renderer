#pragma once

#include <vector>
#include <functional>
#include <string>

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

	Poly3D portal;

	int leafIndex;
	int frontIndex;
	int backIndex;
};

struct Leaf
{
	IndicesList indices;
};

using LeafList = std::vector<Leaf>;

enum class Side
{
	Front,
	Back,
	Coplanar
};

enum class SplitResult
{
	Front,
	Back,
	Coplanar,
	Intersected,
};

struct BoundingBox
{
	Vec3 min, max;
};

struct SplittedPolygon
{
	SplitResult result;
	Poly3D front, back, coplanar;
};

using NodeList = std::vector<Node>;

class BSP
{
public:
	
	void compile(const Poly3DList& polys);
	void render(const Camera& cam, World& world);
	int leaf(const Camera& cam);

	Poly3DList portalPolys;

private:
	int rootNodeIndex = 0;
	NodeList nodes;
	LeafList leaves;
	Poly3DList list;

	int addToList(const Poly3D& poly);
	int addLeaf(const Leaf& leaf);
	Poly3D getFromList(int index);

	int findSplitter(const Poly3DList& polys);
	int build(const Poly3DList& polys, const PlaneList& bounds);
	void traverse(int nodeIndex, const Camera& cam, World& world);
	Side test(const Plane& plane, const Vec3& vec);
	Vertex3D intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane);
	SplittedPolygon splitAgainstPlane(const Poly3D& in, const Plane& plane);

	Poly3D makePolygonOnPlane(const Plane& plane);
	Poly3D clipPolygon(const Poly3D& poly, const PlaneList& bounds);

	BoundingBox makeBoundingBox(const Poly3DList& polys);
	PlaneList getPlanes(const BoundingBox& bbox); 
};

