#pragma once

#include <vector>

#include "Vector.h"
#include "Triangle.h"

struct Viewport
{
	float left;
	float right;
	float top;
	float bottom;

	Viewport() {}

	Viewport(float left, float right, float top, float bottom)
		: left(left), right(right), top(top), bottom(bottom)
	{
	}
};

class ViewportClipper
{
public:
	Poly2D			clip(const Poly2D& input, const Viewport& viewport);
	Triangle2DList	clip(const Triangle2& tri, const Viewport& viewport);

private:
	enum ClipSide;

	bool		inside(const Vertex2D& v, ClipSide side, const Viewport& viewport);
	Vertex2D	intersect(const Vertex2D& v1, const Vertex2D& v2, ClipSide side, const Viewport& viewport);
	void		snapToClipSide(Vertex2D& v, ClipSide side, const Viewport& viewport);
	Poly2D		clipAgainstPlane(const Poly2D& input, ClipSide side, const Viewport& viewport);
};

