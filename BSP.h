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
	bool isLeaf;
	Plane plane;
	IndicesList coplanar;
	IndicesList indices;

	int frontIndex;
	int backIndex;
};

enum Side
{
	Front,
	Back,
	Coplane
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

private:
	int rootNodeIndex = 0;
	NodeList nodes;
	Triangle3DList list;

	int addToList(const Triangle3& tri);
	Triangle3 getFromList(int index);

	int findSplitter(const Triangle3DList& tris);
	int build(const Triangle3DList& tris);
	void traverse(int nodeIndex, const Camera& cam, World& world);
	SplittedTriangle splitTriangle(const Triangle3& tri, const Plane& plane);
	Side test(const Plane& plane, const Vec3& vec);
	Vertex3D intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane);
	SplittedPolygon splitAgainstPlane(const Polygon3D& input, const Plane& plane);
};

