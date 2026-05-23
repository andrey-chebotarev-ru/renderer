#include "Clipper.h"

enum Clipper::ClipPlane
{
	Near,
	Far
};

bool Clipper::inside(const Vertex3D& v, ClipPlane plane, float distance)
{
	switch (plane) {
	case Near:
		return (v.position.z >= distance);
	case Far:
		return (v.position.z <= distance);
	}
}

Vertex3D Clipper::intersect(const Vertex3D& v1, const Vertex3D& v2, float distance)
{
	float t = (distance - v1.position.z) / (v2.position.z - v1.position.z);;

	Vertex3D v = Vertex3D::lerp(v1, v2, t);
	v.position.z = distance;

	return v;
}

Poly3D Clipper::clipAgainstPlane(const Poly3D& in, ClipPlane plane, float distance)
{
	Poly3D out;

	out.texId = in.texId;
	out.fill = in.fill;

	if (in.verts.empty()) {
		return out;
	}

	const Vertex3D* previous = &in.verts.back();
	bool previousInside = inside(*previous, plane, distance);

	for (const auto& current : in.verts) {
		bool currentInside = inside(current, plane, distance);

		if (previousInside && currentInside) {
			out.verts.push_back(current);
		}
		else if (previousInside && !currentInside) {
			out.verts.push_back(intersect(*previous, current, distance));
		}
		else if (!previousInside && currentInside) {
			out.verts.push_back(intersect(*previous, current, distance));
			out.verts.push_back(current);
		}

		previous = &current;
		previousInside = currentInside;
	}

	return out;
}

Poly3D Clipper::clip(const Poly3D& in, float nearDistance, float farDistance)
{
	Poly3D out;

	out = clipAgainstPlane(in, ClipPlane::Near, nearDistance);
	out = clipAgainstPlane(out, ClipPlane::Far, farDistance);

	return out;
}

Triangle3DList Clipper::clip(const Triangle3& in, float nearDistance, float farDistance)
{
	Poly3D out(in);

	out = clip(out, nearDistance, farDistance);

	if (out.verts.size() < 3) {
		return {};
	}

	return out.triangulate();
}