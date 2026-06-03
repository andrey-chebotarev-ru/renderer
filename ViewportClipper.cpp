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

bool ViewportClipper::inside(const Vertex2D& v, ClipSide side, const Viewport& viewport) const
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

Vertex2D ViewportClipper::intersect(const Vertex2D& v1, const Vertex2D& v2, ClipSide side, const Viewport& viewport) const
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

void ViewportClipper::snapToClipSide(Vertex2D& v, ClipSide side, const Viewport& viewport) const
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

Poly2D ViewportClipper::clipAgainstPlane(const Poly2D& in, ClipSide side, const Viewport& viewport) const
{
	Poly2D out;

	out.fill = in.fill;

	if (in.verts.empty()) {
		return out;
	}

	for (int edgeIdx = 0; edgeIdx < in.getEdgeCount(); edgeIdx++) {
		const Edge2D& edge = in.getEdge(edgeIdx);

		bool vert1Inside = inside(edge.vert1, side, viewport);
		bool vert2Inside = inside(edge.vert2, side, viewport);

		if (vert1Inside && vert2Inside) {
			out.addVert(edge.vert2);
		}
		else if (vert1Inside && !vert2Inside) {
			out.addVert(intersect(edge.vert1, edge.vert2, side, viewport));
		}
		else if (!vert1Inside && vert2Inside) {
			out.addVert(intersect(edge.vert1, edge.vert2, side, viewport));
			out.addVert(edge.vert2);
		}
	}

	return out;
}

Poly2D ViewportClipper::clip(const Poly2D& in, const Viewport& viewport) const
{
	Poly2D out;

	out = clipAgainstPlane(in, ClipSide::Left, viewport);
	out = clipAgainstPlane(out, ClipSide::Right, viewport);
	out = clipAgainstPlane(out, ClipSide::Top, viewport);
	out = clipAgainstPlane(out, ClipSide::Bottom, viewport);

	return out;
}

Triangle2DList ViewportClipper::clip(const Triangle2& in, const Viewport& viewport) const
{
	Poly2D out(in);
	
	out = clip(out, viewport);

	if (out.verts.size() < 3) {
		return {};
	}

	return out.triangulate();
}