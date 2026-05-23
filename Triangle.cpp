#include "Triangle.h"

Plane Triangle3::getPlane() const
{
	Vec3 normal = Vec3::normalize(Vec3::cross(v1.position - v0.position, v2.position - v0.position));
	float distance = Vec3::dot(normal, v0.position);

	return Plane(normal, distance);
}

Plane Poly3D::getPlane() const
{
	Vec3 normal = Vec3::normalize(Vec3::cross(verts[1].position - verts[0].position, verts[2].position - verts[0].position));
	float distance = Vec3::dot(normal, verts[0].position);

	return Plane(normal, distance);
}

Triangle3DList Poly3D::triangulate() const
{
	Triangle3DList tris;

	if (verts.size() < 3) {
		return tris;
	}

	for (int i = 1; i < verts.size() - 1; i++) {
		Triangle3 tri;

		tri.v0 = verts[0];
		tri.v1 = verts[i];
		tri.v2 = verts[i + 1];
		tri.texId = texId;
		tri.fill = fill;

		tris.push_back(tri);
	}

	return tris;
}

Triangle2DList Poly2D::triangulate() const
{
	Triangle2DList tris;

	if (verts.size() < 3) {
		return tris;
	}

	for (int i = 1; i < verts.size() - 1; i++) {
		Triangle2 tri;

		tri.v0 = verts[0];
		tri.v1 = verts[i];
		tri.v2 = verts[i + 1];
		tri.fill = fill;

		tris.push_back(tri);
	}

	return tris;
}