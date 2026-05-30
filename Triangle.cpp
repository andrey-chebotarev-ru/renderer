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

Edge2D::Edge2D(const Vertex2D& vert1, const Vertex2D& vert2)
	: vert1(vert1), vert2(vert2)
{
	Vec2 dir = vert2.pos - vert1.pos;
	
	line.normal = Vec2::normalize(Vec2(dir.y, -dir.x));
	line.distance = Vec2::dot(line.normal, vert1.pos);
}

Edge3D::Edge3D(const Vertex3D& vert1, const Vertex3D& vert2)
	: vert1(vert1), vert2(vert2)
{
}

Edge2D Poly2D::getEdge(int idx) const
{
	size_t count = getCount();

	idx %= count;

	return Edge2D(verts[idx], verts[(idx + 1) % count]);
}

float Poly2D::getSignedArea() const
{
	float sum = 0.0f;

	for (int i = 0; i < getCount(); i++) {
		Edge2D edge = getEdge(i);

		sum += Vec2::cross(edge.vert1.pos, edge.vert2.pos);
	}

	return sum * 0.5f;
}

float Poly2D::getArea() const
{
	return abs(getSignedArea());
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

Edge3D Poly3D::getEdge(int idx) const
{
	size_t count = getCount();

	idx %= count;

	return Edge3D(verts[idx], verts[(idx + 1) % count]);
}

float Poly3D::getSignedArea() const
{
	Vec3 sum = Vec3(0, 0, 0);

	for (int edgeIdx = 0; edgeIdx < getCount(); edgeIdx++) {
		Edge3D edge = getEdge(edgeIdx);

		sum += Vec3::cross(edge.vert1.position, edge.vert2.position);
	}

	return Vec3::dot(sum, getPlane().normal) * 0.5f;
}

float Poly3D::getArea() const
{
	return abs(getSignedArea());
}

Vec3 Poly3D::getCenter() const
{
	/*
	Vec3 center = Vec3(0.0f, 0.0f, 0.0f);

	for (const auto& vert : verts) {
		center += vert.position;
	}

	center /= (float)getCount();

	return center;
	*/
	
	Vec3 centroid = Vec3(0, 0, 0);
	float totalArea = 0.0f;

	for (int i = 1; i < getCount() - 1; i++)
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
		
		tris.push_back(tri);
	}

	return tris;
}

Poly3D Poly3D::offset(const Poly3D& poly, float distance)
{
	Vec3 normal = poly.getPlane().normal;
	Poly3D out = poly;

	for (int i = 0; i < poly.getCount(); i++) {
		out.verts[i].position += normal * distance;
	}

	return out;
}