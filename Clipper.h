#pragma once

#include "Triangle.h"

class Clipper
{
public:
	Triangle3DList clipTriangle(const Triangle3& triangle, float nearDistance, float farDistance);

private:
	enum ClipPlane;

	bool		inside(const Vertex3D& v, ClipPlane plane, float distance);
	Vertex3D	intersect(const Vertex3D& v1, const Vertex3D& v2, float distance);
	Polygon3D	clipAgainstPlane(const Polygon3D& input, ClipPlane plane, float distance);
	Polygon3D	clipToFrustrum(const Polygon3D& input, float nearDistance, float farDistance);
};