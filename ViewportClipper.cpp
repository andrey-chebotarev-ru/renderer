#include "ViewportClipper.h"

#include "Vector.h"
#include "Triangle.h"

enum ViewportClipper::ClipSide
{
	Left,
	Right,
	Top,
	Bottom
};

bool ViewportClipper::inside(const Vertex2D& v, ClipSide side, const Viewport& viewport)
{
	switch (side) {
	case Left:
		return (v.pos.x >= viewport.left);
	case Right:
		return (v.pos.x <= viewport.right);
	case Top:
		return (v.pos.y >= viewport.top);
	case Bottom:
		return (v.pos.y <= viewport.bottom);
	}
}

Vertex2D ViewportClipper::intersect(const Vertex2D& v1, const Vertex2D& v2, ClipSide side, const Viewport& viewport)
{
	float t = 0.0f;

	switch (side) {
	case Left:
		t = (viewport.left - v1.pos.x) / (v2.pos.x - v1.pos.x);
		break;
	case Right:
		t = (viewport.right - v1.pos.x) / (v2.pos.x - v1.pos.x);
		break;
	case Top:
		t = (viewport.top - v1.pos.y) / (v2.pos.y - v1.pos.y);
		break;
	case Bottom:
		t = (viewport.bottom - v1.pos.y) / (v2.pos.y - v1.pos.y);
		break;
	}

	Vertex2D v = Vertex2D::lerp(v1, v2, t);

	snapToClipSide(v, side, viewport);

	return v;
}

void ViewportClipper::snapToClipSide(Vertex2D& v, ClipSide side, const Viewport& viewport)
{
	switch (side) {
	case Left:
		v.pos.x = viewport.left;
		break;
	case Right:
		v.pos.x = viewport.right;
		break;
	case Top:
		v.pos.y = viewport.top;
		break;
	case Bottom:
		v.pos.y = viewport.bottom;
		break;
	}
}

Polygon2D ViewportClipper::clipAgainstPlane(const Polygon2D& input, ClipSide side, const Viewport& viewport)
{
	Polygon2D output;

	if (input.empty()) {
		return output;
	}

	const Vertex2D* previous = &input.back();
	bool previousInside = inside(*previous, side, viewport);

	for (const Vertex2D& current : input) {
		bool currentInside = inside(current, side, viewport);

		if (previousInside && currentInside) {
			output.push_back(current);
		}
		else if (previousInside && !currentInside) {
			output.push_back(intersect(*previous, current, side, viewport));
		}
		else if (!previousInside && currentInside) {
			output.push_back(intersect(*previous, current, side, viewport));
			output.push_back(current);
		}

		previous = &current;
		previousInside = currentInside;
	}

	return output;
}

Polygon2D ViewportClipper::clipToViewport(const Polygon2D& input, const Viewport& viewport)
{
	Polygon2D output;

	output = clipAgainstPlane(input, ClipSide::Left, viewport);
	output = clipAgainstPlane(output, ClipSide::Right, viewport);
	output = clipAgainstPlane(output, ClipSide::Top, viewport);
	output = clipAgainstPlane(output, ClipSide::Bottom, viewport);

	return output;
}

Triangle2DList ViewportClipper::clipTriangle(const Triangle2& triangle, const Viewport& viewport)
{
	Triangle2DList clippedTriangles;
	Polygon2D polygon;
	
	clippedTriangles.reserve(5);
	polygon.reserve(8);

	polygon.push_back(triangle.v0);
	polygon.push_back(triangle.v1);
	polygon.push_back(triangle.v2);

	polygon = clipToViewport(polygon, viewport);

	if (polygon.size() < 3) {
		return clippedTriangles;
	}

	for (int i = 1; i < polygon.size() - 1; i++) {
		Triangle2 clippedTriangle;

		clippedTriangle.v0 = polygon[0];
		clippedTriangle.v1 = polygon[i];
		clippedTriangle.v2 = polygon[i + 1];

		clippedTriangle.fill = triangle.fill;

		clippedTriangles.push_back(clippedTriangle);
	}

	return clippedTriangles;
}