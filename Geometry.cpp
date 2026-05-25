#include "Geometry.h"

const float Geometry::epsilon = 0.0001f;

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
	if (edgeIdx >= cutter.getSize()) {
		return;
	}

	Edge2D cutterEdge = cutter.getEdge(edgeIdx);

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

Poly2DList Geometry::subtract(const Poly2D& in, const Poly2D& cutter, float minArea)
{
	Poly2DList out;

	if (!in.isValid()) {
		return out;
	}

	if (!satOverlap(in, cutter)) {
		out.push_back(in);
		return out;
	}

	subtractPoly(in, cutter, 0, out, minArea);

	return out;
}

void Geometry::satProjectPoly(const Poly2D& poly, const Vec2& axis, float& outMin, float& outMax)
{
	outMin = outMax = Vec2::dot(poly.verts[0].pos, axis);

	for (int i = 1; i < poly.getSize(); i++) {
		float proj = Vec2::dot(poly.verts[i].pos, axis);

		outMin = std::min(outMin, proj);
		outMax = std::max(outMax, proj);
	}
}

bool Geometry::satOverlap(const Poly2D& poly1, const Poly2D& poly2)
{
	auto testAxes = [&](const Poly2D& poly) -> bool {
		for (int i = 0; i < poly.getSize(); i++) {
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

Side Geometry::test(const Plane& plane, const Vec3& vec)
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

Vertex3D Geometry::intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane)
{
	float dist1 = Math::distanceToPlane(plane, vert1.position);
	float dist2 = Math::distanceToPlane(plane, vert2.position);

	float t = dist1 / (dist1 - dist2);

	return Vertex3D::lerp(vert1, vert2, t);
}

SplittedPoly3D Geometry::splitAgainstPlane(const Poly3D& in, const Plane& plane)
{
	SplittedPoly3D splittedPoly;

	splittedPoly.front.texId = splittedPoly.back.texId = in.texId;
	splittedPoly.front.fill = splittedPoly.back.fill = in.fill;

	if (in.verts.empty()) {
		return splittedPoly;
	}

	const Vertex3D* previous = &in.verts.back();
	Side previousSide = test(plane, previous->position);

	int frontCount = 0, backCount = 0;

	for (const auto& vert : in.verts) {
		Side side = test(plane, vert.position);

		if (side == Side::Front) {
			frontCount++;
		}
		else if (side == Side::Back) {
			backCount++;
		}
	}

	// Coplanar
	if (frontCount == 0 && backCount == 0) {
		splittedPoly.result = SplitResult3D::Coplanar;
		splittedPoly.coplanar = in;
		return splittedPoly;
	}
	// Front
	else if (backCount == 0) {
		splittedPoly.result = SplitResult3D::Front;
		splittedPoly.front = in;
		return splittedPoly;
	}
	// Back
	else if (frontCount == 0) {
		splittedPoly.result = SplitResult3D::Back;
		splittedPoly.back = in;
		return splittedPoly;
	}

	for (const auto& current : in.verts) {
		Side currentSide = test(plane, current.position);

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
			Vertex3D intersection = intersect(*previous, current, plane);

			splittedPoly.front.verts.push_back(intersection);
			splittedPoly.back.verts.push_back(intersection);
		}

		previous = &current;
		previousSide = currentSide;
	}

	splittedPoly.result = SplitResult3D::Intersected;

	return splittedPoly;
}