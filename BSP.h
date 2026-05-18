#pragma once

#include "Vector.h"
#include "Triangle.h"
#include "Math.h"
#include "World.h"
#include "Camera.h"

struct Node
{
	Triangle3 tri;

	Node* front = nullptr;
	Node* back = nullptr;
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

class BSP
{
public:
	SplittedTriangle splitTriangle(const Triangle3& tri, const Plane& plane);
	Side test(const Plane& plane, const Vec3& vec);
	Node* compile(const Triangle3DList& tris);
	void traverse(Node* node, const Camera& cam, World& world);

private:
	Vertex3D intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane);
	SplittedPolygon splitAgainstPlane(const Polygon3D& input, const Plane& plane);
};

