#pragma once

#include "Triangle.h"

class Clipper
{
public:
	Poly3D			clip(const Poly3D& in, float nearDistance, float farDistance);
	Triangle3DList	clip(const Triangle3& in, float nearDistance, float farDistance);

private:
	enum ClipPlane;

	bool		inside(const Vertex3D& v, ClipPlane plane, float distance);
	Vertex3D	intersect(const Vertex3D& v1, const Vertex3D& v2, float distance);
	Poly3D		clipAgainstPlane(const Poly3D& in, ClipPlane plane, float distance);
};