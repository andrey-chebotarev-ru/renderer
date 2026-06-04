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

class PlaneBasis
{
public:

	PlaneBasis(const Vec3& origin, const Vec3& u, const Vec3& v);

	Vec2 project(const Vec3& vec);
	Poly2D project(const Poly3D& poly);

	Vec3 unproject(const Vec2& vec);
	Poly3D unproject(const Poly2D& poly);

private:

	Vec3 origin;
	Vec3 u, v;
};

class Geometry
{
public:
	static const float epsilon;

	static void makeCW(Poly2D& poly);
	static void makeCCW(Poly2D& poly);
	static Side test(const Line& line, const Vec2& vec);
	static Vertex2D intersect(const Vertex2D& vert1, const Vertex2D& vert2, const Line& line);
	static SplittedPoly2D splitAgainstLine(const Poly2D& in, const Line& line);
	static Poly2DList subtract(Poly2D in, Poly2D cutter, float minArea);
	static Poly3DList subtract(const Poly3D& in, const Poly3D& cutter, const Plane& plane, float minArea);
	static bool satOverlap(const Poly2D& poly1, const Poly2D& poly2);

	static void makeCW(Poly3D& poly);
	static void makeCCW(Poly3D& poly);
	static Side test(const Vec3& vec, const Plane& plane);
	static SplitResult3D classify(const Poly3D& poly, const Plane& plane);
	static Vertex3D intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane);
	static SplittedPoly3D split(const Poly3D& in, const Plane& plane);
	static Poly3D clip(const Poly3D& poly, const Plane& plane);
	static Poly3D clip(const Poly3D& poly, const PlaneList& bounds);
	static Vec3 makePerpendicular(const Vec3& vec);
	static PlaneBasis makePlaneBasis(const Plane& plane);

private:
	static void subtractPoly(const Poly2D& poly, const Poly2D& cutter, int edgeIdx, Poly2DList& out, float minArea);
	static void satProjectPoly(const Poly2D& poly, const Vec2& axis, float& outMin, float& outMax);

};

