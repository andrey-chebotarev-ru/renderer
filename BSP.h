#pragma once

#include "Vector.h"
#include "Triangle.h"
#include "Math.h"

class Node
{
	bool isLeaf;

	Plane plane;

	Node* front;
	Node* back;

	Triangle3DList tris;
};

enum PlaneSide
{
	Front,
	Back,
	On
};

class BSP
{
public:
	PlaneSide test(const Triangle3& tri, const Vec3& vec);
	Node* compile(const Triangle3DList& tris);
};

