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

Vec3 Poly3D::getCenter() const
{
	Vec3 center = Vec3(0.0f, 0.0f, 0.0f);

	for (const auto& vert : verts) {
		center += vert.position;
	}

	center /= (float)getSize();

	return center;


	/*
	Vec3 centroid = Vec3(0, 0, 0);
	float totalArea = 0.0f;

	for (int i = 1; i < getSize() - 1; i++)
	{
		Vec3 a = verts[0].position;
		Vec3 b = verts[i].position;
		Vec3 c = verts[i + 1].position;

		Vec3 triCenter = (a + b + c) / 3.0f;

		float area = Vec3::cross(b - a, c - a).length() * 0.5f;

		centroid += triCenter * area;
		totalArea += area;
	}

	return centroid / totalArea;
	*/
}

Poly3D Poly3D::offset(const Poly3D& poly, float distance)
{
	Vec3 normal = poly.getPlane().normal;
	Poly3D out = poly;

	for (int i = 0; i < poly.getSize(); i++) {
		out.verts[i].position += normal * distance;
	}

	return out;
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