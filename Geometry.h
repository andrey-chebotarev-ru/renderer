#pragma once

#include "Vector.h"
#include "Vertex.h"
#include "Triangle.h"
#include "Math.h"

enum class Side
{
	Front,
	Back,
	Coplanar
};

enum class SplitResult2D
{
	Front,
	Back,
	Colinear,
	Intersected,
};

enum class SplitResult3D
{
	Front,
	Back,
	Coplanar,
	Intersected,
};

struct SplittedPoly2D
{
	SplitResult2D result;
	Poly2D front, back, colinear;
};

struct SplittedPoly3D
{
	SplitResult3D result;
	Poly3D front, back, coplanar;
};


class Geometry
{
public:
	static const float epsilon;

	static Side test(const Line& line, const Vec2& vec);
	static Vertex2D intersect(const Vertex2D& vert1, const Vertex2D& vert2, const Line& line);
	static SplittedPoly2D splitAgainstLine(const Poly2D& in, const Line& line);
	static Poly2DList subtract(const Poly2D& poly, const Poly2D& cutter, float minArea);
	static bool satOverlap(const Poly2D& poly1, const Poly2D& poly2);

	static Side test(const Plane& plane, const Vec3& vec);
	static Vertex3D intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane);
	static SplittedPoly3D splitAgainstPlane(const Poly3D& in, const Plane& plane);

private:
	static void subtractPoly(const Poly2D& poly, const Poly2D& cutter, int edgeIdx, Poly2DList& out, float minArea);
	static void satProjectPoly(const Poly2D& poly, const Vec2& axis, float& outMin, float& outMax);

};

