#include "Vertex.h"

#include "Math.h"

Vertex2D Vertex2D::lerp(const Vertex2D& v1, const Vertex2D& v2, float t)
{
	Vertex2D v;

	v.pos = Vec2::lerp(v1.pos, v2.pos, t);
	v.uz = Math::lerp(v1.uz, v2.uz, t);
	v.vz = Math::lerp(v1.vz, v2.vz, t);
	v.iz = Math::lerp(v1.iz, v2.iz, t);

	return v;
}

Vertex3D Vertex3D::lerp(const Vertex3D& v1, const Vertex3D& v2, float t)
{
	Vertex3D v;

	v.pos = Vec3::lerp(v1.pos, v2.pos, t);
	v.uv = Vec2::lerp(v1.uv, v2.uv, t);

	return v;
}