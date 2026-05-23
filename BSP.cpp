#include "BSP.h"

#include <cstdlib>

#include <SDL3/SDL.h>

Side BSP::test(const Plane& plane, const Vec3& vec)
{
	float dist = Math::distanceToPlane(plane, vec);
	const float epsilon = 0.0001f;

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

static Color changeColor(const Color& c, int j)
{
	Color out = c;
	if (out.r >= 128) {
		out.r -= j;
	}
	else {
		out.r += j;
	}

	if (out.g >= 128) {
		out.g -= j;
	}
	else {
		out.g += j;
	}

	if (out.b >= 128) {
		out.b -= j;
	}
	else {
		out.b += j;
	}

	return out;
}

Vertex3D BSP::intersect(const Vertex3D& vert1, const Vertex3D& vert2, const Plane& plane)
{
	float dist1 = Math::distanceToPlane(plane, vert1.position);
	float dist2 = Math::distanceToPlane(plane, vert2.position);

	float t = dist1 / (dist1 - dist2);

	return Vertex3D::lerp(vert1, vert2, t);
}

SplittedPolygon BSP::splitAgainstPlane(const Poly3D& in, const Plane& plane)
{
	SplittedPolygon splittedPoly;

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
		splittedPoly.result = SplitResult::Coplanar;
		splittedPoly.coplanar = in;
		return splittedPoly;
	}
	// Front
	else if (backCount == 0) {
		splittedPoly.result = SplitResult::Front;
		splittedPoly.front = in;
		return splittedPoly;
	}
	// Back
	else if (frontCount == 0) {
		splittedPoly.result = SplitResult::Back;
		splittedPoly.back = in;
		return splittedPoly;
	}

	for (const Vertex3D& current : in.verts) {
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

	splittedPoly.result = SplitResult::Intersected;

	return splittedPoly;
}

int BSP::findSplitter(const Poly3DList& polys)
{
	int index = -1;

	if (polys.size() <= 1) {
		return index;
	}

	int minScore = polys.size() * 2;

	for (int j = 0; j < polys.size(); j++) {
		const Poly3D& splitter = polys[j];

		int frontCount = 0, backCount = 0;

		for (int i = 0; i < polys.size(); i++) {
			if (i == j) {
				continue;
			}

			SplittedPolygon splitted = splitAgainstPlane(polys[i], splitter.getPlane());

			if (splitted.result == SplitResult::Front) {
				frontCount++;
			}
			else if (splitted.result == SplitResult::Back) {
				backCount++;
			}
			else if (splitted.result == SplitResult::Intersected) {
				frontCount++;
				backCount++;
			}
		}

		if (frontCount > 0 && backCount > 0) {
			int score = abs((int)(backCount - frontCount));
			if (score < minScore) {
				index = j;
				minScore = score;
			}
		}
	}

	return index;
}

int BSP::addToList(const Poly3D& poly)
{
	list.push_back(poly);
	return list.size() - 1;
}

int BSP::addLeaf(const Leaf& leaf)
{
	leaves.push_back(leaf);
	return leaves.size() - 1;
}

void BSP::compile(const Poly3DList& polys)
{
	nodes.clear();
	list.clear();

	BoundingBox worldBBox = makeBoundingBox(polys);
	PlaneList bounds = getPlanes(worldBBox);

	rootNodeIndex = build(polys, bounds);
}

void BSP::render(const Camera& cam, World& world)
{
	traverse(rootNodeIndex, cam, world);
}

int BSP::build(const Poly3DList& polys, const PlaneList& bounds)
{
	if (polys.size() == 0) {
		return -1;
	}

	Node node;

	int splitterIndex = findSplitter(polys);

	if (splitterIndex != -1) {
		Poly3DList front, back;
		Poly3D splitter = polys[splitterIndex];
		node.leafIndex = -1;
		node.plane = splitter.getPlane();
		splitter.fill = Color(255, 255, 255);
		node.coplanar.push_back(addToList(splitter));

		node.portal = makePolygonOnPlane(node.plane);
		node.portal = clipPolygon(node.portal, bounds);
		node.portal.fill = Color();
		node.portal.texId = 1;

		Poly3D portal = node.portal;
		Vec3 normal = portal.getPlane().normal;
		for (auto& vert : portal.verts) {
			vert.position += normal * 0.1f;
		}
		portalPolys.push_back(portal);

		for (int i = 0; i < polys.size(); i++) {

			if (i == splitterIndex) {
				continue;
			}

			
			SplittedPolygon splittedPoly = splitAgainstPlane(polys[i], node.plane);

			if (splittedPoly.result == SplitResult::Coplanar) {
				Poly3D coplanarPoly = splittedPoly.coplanar;
				coplanarPoly.fill = Color(255, 255, 255);
				node.coplanar.push_back(addToList(coplanarPoly));
			}
			else if (splittedPoly.result == SplitResult::Front) {
				front.push_back(splittedPoly.front);
			}
			else if (splittedPoly.result == SplitResult::Back) {
				back.push_back(splittedPoly.back);
			}
			else if (splittedPoly.result == SplitResult::Intersected) {
				front.push_back(splittedPoly.front);
				back.push_back(splittedPoly.back);
			}
		}

		PlaneList frontBounds = bounds;
		frontBounds.push_back(node.plane);

		PlaneList backBounds = bounds;
		backBounds.push_back(node.plane.flip());


		node.frontIndex = build(front, frontBounds);
		node.backIndex = build(back, backBounds);

	}
	else {
		Leaf leaf;

		node.frontIndex = -1;
		node.backIndex = -1;

		Color c;
		for (int j = 0; j < polys.size(); j++) {
			Poly3D poly = polys[j];
			poly.fill = changeColor(c, j);
			leaf.indices.push_back(addToList(poly));
		}

		node.leafIndex = addLeaf(leaf);
	}

	nodes.push_back(node);

	return nodes.size() - 1;
}

void BSP::traverse(int nodeIndex, const Camera& cam, World& world)
{
	if (nodeIndex == -1) {
		return;
	}

	const Node& node = nodes[nodeIndex];

	if (node.leafIndex != -1)
	{
		Leaf leaf = leaves[node.leafIndex];

		for (int index : leaf.indices) {
			world.addGeometry(list[index]);
		}
	}
	else
	{
		Side side = test(node.plane, cam.position);

		if (side == Side::Front) {
			traverse(node.backIndex, cam, world);

			for (int index : node.coplanar) {
				world.addGeometry(list[index]);
			}
			
			traverse(node.frontIndex, cam, world);
		}
		else if (side == Side::Back) {
			traverse(node.frontIndex, cam, world);

			for (int index : node.coplanar) {
				world.addGeometry(list[index]);
			}

			traverse(node.backIndex, cam, world);
		}
		else {
			traverse(node.frontIndex, cam, world);
			traverse(node.backIndex, cam, world);
		}

	}
}

int BSP::leaf(const Camera& cam)
{
	int nodeId = rootNodeIndex;
	while (true) {
		const Node& node = nodes[nodeId];

		if (node.leafIndex != -1) {
			return node.leafIndex;
		}

		if (test(node.plane, cam.position) == Side::Back) {
			nodeId = node.backIndex;
		}
		else {
			nodeId = node.frontIndex;
		}
	}

	return 0;
}

// AI
Poly3D BSP::makePolygonOnPlane(const Plane& plane)
{
	Vec3 center = plane.normal * plane.distance;

	Vec3 temp = fabs(plane.normal.y) < 0.99f ? Vec3(0, 1, 0) : Vec3(1, 0, 0);

	Vec3 axisU = Vec3::normalize(Vec3::cross(temp, plane.normal));
	Vec3 axisV = Vec3::normalize(Vec3::cross(plane.normal, axisU));

	constexpr float size = 1000.0f;

	Poly3D poly;

	poly.texId = 1;
	poly.fill = Color();

	poly.verts.push_back(Vertex3D(center - axisU * size - axisV * size, Vec2(0, 0)));
	poly.verts.push_back(Vertex3D(center + axisU * size - axisV * size, Vec2(1, 0)));
	poly.verts.push_back(Vertex3D(center + axisU * size + axisV * size, Vec2(1, 1)));
	poly.verts.push_back(Vertex3D(center - axisU * size + axisV * size, Vec2(0, 1)));

	return poly;
}

Poly3D BSP::clipPolygon(const Poly3D& in, const PlaneList& bounds)
{
	Poly3D out = in;

	for (const Plane& plane : bounds)
	{
		SplittedPolygon splittedPoly = splitAgainstPlane(out, plane);

		if (splittedPoly.result == SplitResult::Front || splittedPoly.result == SplitResult::Intersected) {
			out = splittedPoly.front;
		}
	}

	return out;
}

BoundingBox BSP::makeBoundingBox(const Poly3DList& polys)
{
	BoundingBox bbox;

	if (polys.size() == 0) {
		return bbox;
	}

	// FIX

	bbox.min = bbox.max = polys[0].verts[0].position;

	for (const auto& poly : polys) {

		for (const auto& vert : poly.verts) {
			bbox.min.x = std::min(bbox.min.x, vert.position.x);
			bbox.min.y = std::min(bbox.min.y, vert.position.y);
			bbox.min.z = std::min(bbox.min.z, vert.position.z);

			bbox.max.x = std::max(bbox.max.x, vert.position.x);
			bbox.max.y = std::max(bbox.max.y, vert.position.y);
			bbox.max.z = std::max(bbox.max.z, vert.position.z);
		}
	}

	return bbox;
}

PlaneList BSP::getPlanes(const BoundingBox& bbox)
{
	PlaneList planes;

	planes.push_back(Plane(Vec3(-1, 0, 0), -bbox.max.x));
	planes.push_back(Plane(Vec3(1, 0, 0), bbox.min.x));

	planes.push_back(Plane(Vec3(0, -1, 0), -bbox.max.y));
	planes.push_back(Plane(Vec3(0, 1, 0), bbox.min.y));

	planes.push_back(Plane(Vec3(0, 0, -1), -bbox.max.z));
	planes.push_back(Plane(Vec3(0, 0, 1), bbox.min.z));

	return planes;
}