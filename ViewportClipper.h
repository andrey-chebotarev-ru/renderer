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
	Triangle2DList clipTriangle(const Triangle2& triangle, const Viewport& viewport);

private:
	enum ClipSide;

	bool		inside(const Vertex2D& v, ClipSide side, const Viewport& viewport);
	Vertex2D	intersect(const Vertex2D& v1, const Vertex2D& v2, ClipSide side, const Viewport& viewport);
	void		snapToClipSide(Vertex2D& v, ClipSide side, const Viewport& viewport);
	Polygon2D	clipAgainstPlane(const Polygon2D& input, ClipSide side, const Viewport& viewport);
	Polygon2D	clipToViewport(const Polygon2D& input, const Viewport& viewport);
};

