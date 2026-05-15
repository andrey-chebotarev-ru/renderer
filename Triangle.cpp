#include "Triangle.h"

Vec3 Triangle3::getNormal()
{
	return Vec3::normalize(Vec3::cross(v1.position - v0.position, v2.position - v0.position));
}
