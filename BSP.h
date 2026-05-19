#pragma once

#include <vector>

#include "Vector.h"
#include "Triangle.h"
#include "Math.h"
#include "World.h"
#include "Camera.h"

struct Node
{
	bool isLeaf;
	Plane plane;
	Triangle3DList tris;
	
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
	Polygon3D front;
	Polygon3D back;
};

struct SplittedTriangle
{
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
	int rootNode = 0;
	NodeList nodes;

	int findSplitter(const Triangle3DList& tris);
	int build(const Triangle3DList& tris);
	void traverse(int nodeIndex, const Camera& cam, World& world);
	SplittedTriangle splitTriangle(const Triangle3& tri, const Plane& plane);
	Side test(const Plane& plane, const Vec3& vec);
	Vertex3D intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane);
	SplittedPolygon splitAgainstPlane(const Polygon3D& input, const Plane& plane);
};

