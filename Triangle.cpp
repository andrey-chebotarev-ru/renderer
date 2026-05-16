#include "Triangle.h"

Plane Triangle3::getPlane() const
{
	Vec3 normal = Vec3::normalize(Vec3::cross(v1.position - v0.position, v2.position - v0.position));
	float distance = Vec3::dot(normal, v0.position);

	return Plane(normal, distance);
}

float Triangle3::test(const Vec3& vec) const
{
	Plane p = getPlane();
	return Vec3::dot(p.normal, vec) - p.distance;
}
