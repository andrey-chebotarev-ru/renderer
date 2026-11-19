#include "Clipper.h"

enum Clipper::ClipPlane
{
	Near,
	Far
};

bool Clipper::inside(const Vertex3D& v, ClipPlane plane, float distance) const
{
	switch (plane) {
	case Near:
		return (v.pos.z >= distance);
	case Far:
		return (v.pos.z <= distance);
	}
}

Vertex3D Clipper::intersect(const Vertex3D& v1, const Vertex3D& v2, float distance) const
{
	float t = (distance - v1.pos.z) / (v2.pos.z - v1.pos.z);;

	Vertex3D v = Vertex3D::lerp(v1, v2, t);
	v.pos.z = distance;

	return v;
}

Poly3D Clipper::clipAgainstPlane(const Poly3D& in, ClipPlane plane, float distance) const
{
	Poly3D out;

	if (!in.isValid()) {
		return out;
	}

	for (int edgeIdx = 0; edgeIdx < in.getEdgeCount(); edgeIdx++) {
		const Edge3D& edge = in.getEdge(edgeIdx);
		
		bool vert1Inside = inside(edge.vert1, plane, distance);
		bool vert2Inside = inside(edge.vert2, plane, distance);

		if (vert1Inside && vert2Inside) {
			out.addVert(edge.vert2);
		}
		else if (vert1Inside && !vert2Inside) {
			out.addVert(intersect(edge.vert1, edge.vert2, distance));
		}
		else if (!vert1Inside && vert2Inside) {
			out.addVert(intersect(edge.vert1, edge.vert2, distance));
			out.addVert(edge.vert2);
		}
	}

	return out;
}

Poly3D Clipper::clip(const Poly3D& in, float nearDistance, float farDistance) const
{
	Poly3D out;

	out = clipAgainstPlane(in, ClipPlane::Near, nearDistance);
	out = clipAgainstPlane(out, ClipPlane::Far, farDistance);

	return out;
}

Triangle3DList Clipper::clip(const Triangle3& in, float nearDistance, float farDistance) const
{
	Poly3D out(in);

	out = clip(out, nearDistance, farDistance);

	if (out.verts.size() < 3) {
		return {};
	}

	return out.triangulate();
}