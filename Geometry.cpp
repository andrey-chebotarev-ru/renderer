#include "Geometry.h"

#include <iostream>

const float Geometry::epsilon = 0.05f;

PlaneBasis::PlaneBasis(const Vec3& origin, const Vec3& u, const Vec3& v)
	: origin(origin), u(u), v(v) {
}

Vec2 PlaneBasis::project(const Vec3& p)
{
	Vec3 q = p - origin;

	return Vec2(Vec3::dot(q, u), Vec3::dot(q, v));
}

Poly2D PlaneBasis::project(const Poly3D& poly)
{
	Poly2D proj;

	for (const auto& vert : poly.verts) {
		Vec2 vec = project(vert.pos);
		proj.addVert(Vertex2D(vec, vert.uv.x, vert.uv.y, 1.0f));
	}

	return proj;
}

Vec3 PlaneBasis::unproject(const Vec2& p)
{
	return origin + u * p.x + v * p.y;
}

Poly3D PlaneBasis::unproject(const Poly2D& poly)
{
	Poly3D unproj;

	for (const auto& vert : poly.verts) {
		Vec3 vec = unproject(vert.pos);
		unproj.addVert(Vertex3D(vec, Vec2(vert.uz, vert.vz)));
	}

	return unproj;
}

void Geometry::makeCW(Poly2D& poly)
{
	if (poly.getSignedArea() >= 0.0f) {
		std::reverse(poly.verts.begin(), poly.verts.end());
	}
}

void Geometry::makeCCW(Poly2D& poly)
{
	if (poly.getSignedArea() < 0.0f) {
		std::reverse(poly.verts.begin(), poly.verts.end());
	}
}

Side Geometry::test(const Line& line, const Vec2& vec)
{
	float dist = Math::distanceToLine(line, vec);

	if (dist > epsilon) {
		return Side::Front;
	}
	else if (dist < -epsilon) {
		return Side::Back;
	}
	else {
		return Side::Coplanar;
	}
}

Vertex2D Geometry::intersect(const Vertex2D& vert1, const Vertex2D& vert2, const Line& line)
{
	float dist1 = Math::distanceToLine(line, vert1.pos);
	float dist2 = Math::distanceToLine(line, vert2.pos);

	float t = dist1 / (dist1 - dist2);

	return Vertex2D::lerp(vert1, vert2, t);
}

SplittedPoly2D Geometry::splitAgainstLine(const Poly2D& in, const Line& line)
{
	SplittedPoly2D splittedPoly;

	splittedPoly.front.fill = splittedPoly.back.fill = in.fill;

	splittedPoly.front.fill = Color();
	splittedPoly.back.fill = Color();

	if (in.verts.empty()) {
		return splittedPoly;
	}

	const Vertex2D* previous = &in.verts.back();
	Side previousSide = test(line, previous->pos);

	int frontCount = 0, backCount = 0;

	for (const auto& vert : in.verts) {
		Side side = test(line, vert.pos);

		if (side == Side::Front) {
			frontCount++;
		}
		else if (side == Side::Back) {
			backCount++;
		}
	}

	// Colinear
	if (frontCount == 0 && backCount == 0) {
		splittedPoly.result = SplitResult2D::Colinear;
		splittedPoly.colinear = in;
		return splittedPoly;
	}
	// Front
	else if (backCount == 0) {
		splittedPoly.result = SplitResult2D::Front;
		splittedPoly.front = in;
		return splittedPoly;
	}
	// Back
	else if (frontCount == 0) {
		splittedPoly.result = SplitResult2D::Back;
		splittedPoly.back = in;
		return splittedPoly;
	}

	for (const auto& current : in.verts) {
		Side currentSide = test(line, current.pos);

		if (previousSide == Side::Front) {
			splittedPoly.front.verts.push_back(*previous);
		}
		else if (previousSide == Side::Back) {
			splittedPoly.back.verts.push_back(*previous);
		}
		else if (previousSide == Side::Coplanar) {
			splittedPoly.front.verts.push_back(*previous);
			splittedPoly.back.verts.push_back(*previous);
		}

		if ((previousSide == Side::Front && currentSide == Side::Back) || (previousSide == Side::Back && currentSide == Side::Front)) {
			Vertex2D intersection = intersect(*previous, current, line);

			splittedPoly.front.verts.push_back(intersection);
			splittedPoly.back.verts.push_back(intersection);
		}

		previous = &current;
		previousSide = currentSide;
	}

	splittedPoly.result = SplitResult2D::Intersected;

	return splittedPoly;
}

void Geometry::subtractPoly(const Poly2D& poly, const Poly2D& cutter, int edgeIdx, Poly2DList& out, float minArea)
{
	if (edgeIdx >= cutter.getCount()) {
		return;
	}

	const Edge2D& cutterEdge = cutter.getEdge(edgeIdx);

	SplittedPoly2D splittedPoly = splitAgainstLine(poly, cutterEdge.line);

	if (splittedPoly.front.isValid()) {
		if (splittedPoly.front.getArea() >= minArea) {
			out.push_back(splittedPoly.front);
		}
	}

	if (splittedPoly.back.isValid()) {
		subtractPoly(splittedPoly.back, cutter, edgeIdx + 1, out, minArea);
	}
}

Poly2DList Geometry::subtract(Poly2D in, Poly2D cutter, float minArea)
{
	Poly2DList out;

	if (!in.isValid()) {
		return out;
	}

	if (!satOverlap(in, cutter)) {
		out.push_back(in);
		return out;
	}

	makeCCW(cutter);
	makeCCW(in);

	subtractPoly(in, cutter, 0, out, minArea);

	return out;
}

Poly3DList Geometry::subtract(const Poly3D& in, const Poly3D& cutter, const Plane& plane, float minArea)
{
	Poly3DList out;
	PlaneBasis basis = makePlaneBasis(plane);

	Poly2DList polys = subtract(basis.project(in), basis.project(cutter), minArea);

	for (const auto& poly : polys) {
		out.push_back(basis.unproject(poly));
	}

	return out;
}

void Geometry::satProjectPoly(const Poly2D& poly, const Vec2& axis, float& outMin, float& outMax)
{
	outMin = outMax = Vec2::dot(poly.verts[0].pos, axis);

	for (int i = 1; i < poly.getCount(); i++) {
		float proj = Vec2::dot(poly.verts[i].pos, axis);

		outMin = std::min(outMin, proj);
		outMax = std::max(outMax, proj);
	}
}

bool Geometry::satOverlap(const Poly2D& poly1, const Poly2D& poly2)
{
	auto testAxes = [&](const Poly2D& poly) -> bool {
		for (int i = 0; i < poly.getCount(); i++) {
			Vec2 axis = poly.getEdge(i).line.normal;
			float min1, max1;
			float min2, max2;
			satProjectPoly(poly1, axis, min1, max1);
			satProjectPoly(poly2, axis, min2, max2);
			if (max1 < min2 - epsilon || max2 < min1 - epsilon) {
				return false;
			}
		}
		return true;
		};

	if (!testAxes(poly1) || !testAxes(poly2)) {
		return false;
	}

	return true;
}

void Geometry::makeCW(Poly3D& poly)
{
	if (poly.getSignedArea() >= 0.0f) {
		std::reverse(poly.verts.begin(), poly.verts.end());
	}
}

void Geometry::makeCCW(Poly3D& poly)
{
	if (poly.getSignedArea() < 0.0f) {
		std::reverse(poly.verts.begin(), poly.verts.end());
	}
}

Side Geometry::test(const Vec3& vec, const Plane& plane)
{
	float dist = Math::distanceToPlane(plane, vec);

	if (dist > epsilon) {
		return Side::Front;
	}
	else if (dist < -epsilon) {
		return Side::Back;
	}
	else {
		return Side::Coplanar;
	}
}

/*
* Классификация полигона относительно плоскости
*/
SplitResult3D Geometry::classify(const Poly3D& poly, const Plane& plane)
{
	int frontCount = 0, backCount = 0;

	for (const auto& vert : poly.verts) {
		Side side = test(vert.pos, plane);

		if (side == Side::Front) {
			frontCount++;
		}
		else if (side == Side::Back) {
			backCount++;
		}
	}

	// Все точки лежат на одной плоскости
	if (frontCount == 0 && backCount == 0) {
		return SplitResult3D::Coplanar;
	}
	// Все точки лежат на внутренней стороне
	else if (backCount == 0) {
		return SplitResult3D::Front;
	}
	// Все точки лежат на внешней стороне
	else if (frontCount == 0) {
		return SplitResult3D::Back;
	}
	// Точки по разные стороны - плоскость рассекает полигон
	else {
		return SplitResult3D::Intersected;
	}
}

/*
* Нахождение точки пересечения
*/
Vertex3D Geometry::intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane)
{
	float dist1 = Math::distanceToPlane(plane, vert1.pos);
	float dist2 = Math::distanceToPlane(plane, vert2.pos);

	float t = dist1 / (dist1 - dist2);

	return Vertex3D::lerp(vert1, vert2, t);
}

/*
* Разбиение полигона
*/
SplittedPoly3D Geometry::split(const Poly3D& in, const Plane& plane)
{
	SplittedPoly3D splittedPoly;

	splittedPoly.result = classify(in, plane);

	if (splittedPoly.result == SplitResult3D::Coplanar) {
		splittedPoly.coplanar = in;
		return splittedPoly;
	}

	if (splittedPoly.result == SplitResult3D::Front) {
		splittedPoly.front = in;
		return splittedPoly;
	}

	if (splittedPoly.result == SplitResult3D::Back) {
		splittedPoly.back = in;
		return splittedPoly;
	}

	for (int edgeIdx = 0; edgeIdx < in.getEdgeCount(); edgeIdx++) {
		const Edge3D& edge = in.getEdge(edgeIdx);

		Side vert1Side = test(edge.vert1.pos, plane);
		Side vert2Side = test(edge.vert2.pos, plane);

		if (vert1Side == Side::Front) {
			splittedPoly.front.verts.push_back(edge.vert1);
		}
		else if (vert1Side == Side::Back) {
			splittedPoly.back.verts.push_back(edge.vert1);
		}
		else if (vert1Side == Side::Coplanar) {
			splittedPoly.front.verts.push_back(edge.vert1);
			splittedPoly.back.verts.push_back(edge.vert1);
		}

		if ((vert1Side == Side::Front && vert2Side == Side::Back) || (vert1Side == Side::Back && vert2Side == Side::Front)) {
			Vertex3D intersection = intersect(edge.vert1, edge.vert2, plane);

			splittedPoly.front.verts.push_back(intersection);
			splittedPoly.back.verts.push_back(intersection);
		}
	}

	return splittedPoly;
}

Poly3D Geometry::clip(const Poly3D& in, const Plane& plane)
{
	Poly3D out;

	if (!in.isValid()) {
		return out;
	}

	SplitResult3D result = classify(in, plane);

	if (result == SplitResult3D::Coplanar) {
		return in;
	}

	if (result == SplitResult3D::Front) {
		return in;
	}

	if (result == SplitResult3D::Back) {
		out.clear();
		return out;
	}

	for (int edgeIdx = 0; edgeIdx < in.getEdgeCount(); edgeIdx++) {
		const Edge3D& edge = in.getEdge(edgeIdx);

		bool vert1Inside = (test(edge.vert1.pos, plane) == Side::Front);
		bool vert2Inside = (test(edge.vert2.pos, plane) == Side::Front);

		if (vert1Inside && vert2Inside) {
			out.addVert(edge.vert2);
		}
		else if (vert1Inside && !vert2Inside) {
			out.addVert(intersect(edge.vert1, edge.vert2, plane));
		}
		else if (!vert1Inside && vert2Inside) {
			out.addVert(intersect(edge.vert1, edge.vert2, plane));
			out.addVert(edge.vert2);
		}
	}

	return out;
}

Poly3D Geometry::clip(const Poly3D& in, const PlaneList& bounds)
{
	Poly3D out = in;

	if (!in.isValid()) {
		out.clear();
		return out;
	}

	for (const Plane& plane : bounds)
	{
		out = Geometry::clip(out, plane);

		if (!out.isValid()) {
			out.clear();
			break;
		}
	}

	return out;
}

Vec3 Geometry::makePerpendicular(const Vec3& vec)
{
	if (fabs(vec.x) < fabs(vec.y) && fabs(vec.x) < fabs(vec.z)) {
		return Vec3::normalize(Vec3::cross(vec, Vec3(1, 0, 0)));
	}
	else if (fabs(vec.y) < fabs(vec.z)) {
		return Vec3::normalize(Vec3::cross(vec, Vec3(0, 1, 0)));
	}
	else {
		return Vec3::normalize(Vec3::cross(vec, Vec3(0, 0, 1)));
	}
}

PlaneBasis Geometry::makePlaneBasis(const Plane& plane)
{
	Vec3 origin = plane.normal * plane.distance;

	Vec3 u = Vec3::normalize(makePerpendicular(plane.normal));
	Vec3 v = Vec3::cross(plane.normal, u);

	return PlaneBasis(origin, u, v);
}

Poly3D Geometry::makePolygonOnPlane(const Plane& plane, float size)
{
	Vec3 center = plane.normal * plane.distance;

	Vec3 temp = fabs(plane.normal.y) < 0.99f ? Vec3(0, 1, 0) : Vec3(1, 0, 0);

	Vec3 axisU = Vec3::normalize(Vec3::cross(temp, plane.normal));
	Vec3 axisV = Vec3::normalize(Vec3::cross(plane.normal, axisU));

	Poly3D poly;

	poly.addVert(Vertex3D(center - axisU * size - axisV * size, Vec2(0, 0)));
	poly.addVert(Vertex3D(center + axisU * size - axisV * size, Vec2(1, 0)));
	poly.addVert(Vertex3D(center + axisU * size + axisV * size, Vec2(1, 1)));
	poly.addVert(Vertex3D(center - axisU * size + axisV * size, Vec2(0, 1)));

	return poly;
}

Plane Geometry::makePlane(const Vec3& a, const Vec3& b, const Vec3& c)
{
	Vec3 normal = Vec3::normalize(Vec3::cross(b - a, c - a));
	float distance = Vec3::dot(normal, a);

	return Plane(normal, distance);
}
