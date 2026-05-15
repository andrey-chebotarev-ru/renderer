#include "Clipper.h"

#include "Vector.h"
#include "Triangle.h"

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

Polygon3D Clipper::clipAgainstPlane(const Polygon3D& input, ClipPlane plane, float distance)
{
	Polygon3D output;

	if (input.empty()) {
		return output;
	}

	const Vertex3D* previous = &input.back();
	bool previousInside = inside(*previous, plane, distance);

	for (const Vertex3D& current : input) {
		bool currentInside = inside(current, plane, distance);

		if (previousInside && currentInside) {
			output.push_back(current);
		}
		else if (previousInside && !currentInside) {
			output.push_back(intersect(*previous, current, distance));
		}
		else if (!previousInside && currentInside) {
			output.push_back(intersect(*previous, current, distance));
			output.push_back(current);
		}

		previous = &current;
		previousInside = currentInside;
	}

	return output;
}

Polygon3D Clipper::clipToFrustrum(const Polygon3D& input, float nearDistance, float farDistance)
{
	Polygon3D output;

	output = clipAgainstPlane(input, ClipPlane::Near, nearDistance);
	output = clipAgainstPlane(output, ClipPlane::Far, farDistance);

	return output;
}

Triangle3DList Clipper::clipTriangle(const Triangle3& triangle, float nearDistance, float farDistance)
{
	Triangle3DList clippedTriangles;
	Polygon3D polygon;

	clippedTriangles.reserve(5);
	polygon.reserve(8);

	polygon.push_back(triangle.v0);
	polygon.push_back(triangle.v1);
	polygon.push_back(triangle.v2);

	polygon = clipToFrustrum(polygon, nearDistance, farDistance);

	if (polygon.size() < 3) {
		return clippedTriangles;
	}

	for (int i = 1; i < polygon.size() - 1; i++) {
		Triangle3 clippedTriangle;

		clippedTriangle.v0 = polygon[0];
		clippedTriangle.v1 = polygon[i];
		clippedTriangle.v2 = polygon[i + 1];
		
		clippedTriangles.push_back(clippedTriangle);
	}

	return clippedTriangles;
}
