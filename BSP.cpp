#include "BSP.h"

#include <cstdlib>

#include <SDL3/SDL.h>

Side BSP::test(const Plane& plane, const Vec3& vec)
{
	float dist = Math::distanceToPlane(plane, vec);
	const float epsilon = 0.0001f;

	if (dist > epsilon) {
		return Front;
	}
	else if (dist < -epsilon) {
		return Back;
	}
	else {
		return Coplane;
	}
}

static Color generateColor()
{
	Color c;
	c.r = SDL_rand(256);
	c.g = SDL_rand(256);
	c.b = SDL_rand(256);
	c.a = 255;

	return c;
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

SplittedPolygon BSP::splitAgainstPlane(const Polygon3D& input, const Plane& plane)
{
	SplittedPolygon polygon;

	if (input.empty()) {
		return polygon;
	}

	const Vertex3D* previous = &input.back();
	Side previousSide = test(plane, previous->position);

	int frontCount = 0, backCount = 0;

	for (const Vertex3D& classified : input) {
		Side side = test(plane, classified.position);

		if (side == Front) {
			frontCount++;
		}
		else if (side == Back) {
			backCount++;
		}
	}
	
	// Coplanar
	if (frontCount == 0 && backCount == 0) {
		polygon.coplanar = true;
		return polygon;
	}
	// Front
	else if (backCount == 0) {
		polygon.front.insert(polygon.front.end(), input.begin(), input.end());
		return polygon;
	}
	// Back
	else if (frontCount == 0) {
		polygon.back.insert(polygon.back.end(), input.begin(), input.end());
		return polygon;
	}

	for (const Vertex3D& current : input) {
		Side currentSide = test(plane, current.position);

		if (previousSide == Front) {
			polygon.front.push_back(*previous);
		}
		else if (previousSide == Back) {
			polygon.back.push_back(*previous);
		}
		else if (previousSide == Coplane) {
			polygon.front.push_back(*previous);
			polygon.back.push_back(*previous);
		}
		
		if ((previousSide == Front && currentSide == Back) || (previousSide == Back && currentSide == Front)) {
			Vertex3D intersection = intersect(*previous, current, plane);

			polygon.front.push_back(intersection);
			polygon.back.push_back(intersection);
		}

		previous = &current;
		previousSide = currentSide;
	}

	return polygon;
}

SplittedTriangle BSP::splitTriangle(const Triangle3& tri, const Plane& plane)
{
	SplittedTriangle splittedTri;
	SplittedPolygon splittedPoly;
	Polygon3D poly;

	splittedTri.front.reserve(2);
	splittedTri.back.reserve(2);

	splittedPoly.front.reserve(4);
	splittedPoly.back.reserve(4);

	poly.reserve(3);
	poly.push_back(tri.v0);
	poly.push_back(tri.v1);
	poly.push_back(tri.v2);

	splittedPoly = splitAgainstPlane(poly, plane);

	if (splittedPoly.coplanar) {
		splittedTri.coplanar = true;
		return splittedTri;
	}

	if (splittedPoly.front.size() >= 3) {
		for (int i = 1; i < splittedPoly.front.size() - 1; i++) {
			Triangle3 out;

			out.v0 = splittedPoly.front[0];
			out.v1 = splittedPoly.front[i];
			out.v2 = splittedPoly.front[i + 1];
			out.texId = tri.texId;
			out.fill = generateColor();

			splittedTri.front.push_back(out);
		}
	}
	
	if (splittedPoly.back.size() >= 3) {
		for (int i = 1; i < splittedPoly.back.size() - 1; i++) {
			Triangle3 out;

			out.v0 = splittedPoly.back[0];
			out.v1 = splittedPoly.back[i];
			out.v2 = splittedPoly.back[i + 1];
			out.texId = tri.texId;
			out.fill = generateColor();

			splittedTri.back.push_back(out);
		}
	}
	

	return splittedTri;
}

int BSP::findSplitter(const Triangle3DList& tris)
{
	int index = -1;

	if (tris.size() <= 1) {
		return index;
	}

	

	int minScore = tris.size() * 2;

	for (int j = 0; j < tris.size(); j++) {
		const Triangle3& splitter = tris[j];

		for (int i = 0; i < tris.size(); i++) {
			if (i == j) {
				continue;
			}

			SplittedTriangle splitted = splitTriangle(tris[i], splitter.getPlane());


			if (splitted.back.size() > 0) {
				int score = abs((int)(splitted.back.size() - splitted.front.size()));
				if (score < minScore) {
					index = j;
					minScore = score;
				}
			}
		}
	}

	return index;
}

int BSP::addToList(const Triangle3& tri)
{
	list.push_back(tri);
	return list.size() - 1;
}

int BSP::addLeaf(const Leaf& leaf)
{
	leaves.push_back(leaf);
	return leaves.size() - 1;
}

void BSP::compile(const Triangle3DList& tris)
{
	nodes.clear();
	list.clear();

	PlaneList bounds;

	for (const Triangle3& tri : tris) {
		bounds.push_back(tri.getPlane());
	}

	rootNodeIndex = build(tris, bounds);
}

void BSP::render(const Camera& cam, World& world)
{
	traverse(rootNodeIndex, cam, world);
}

int BSP::build(const Triangle3DList& tris, const PlaneList& bounds)
{
	if (tris.size() == 0) {
		return -1;
	}

	Node node;

	int splitterIndex = findSplitter(tris);

	if (splitterIndex != -1) {
		Triangle3DList front, back;
		Triangle3 splitter = tris[splitterIndex];
		node.leafIndex = -1;
		node.plane = splitter.getPlane();
		splitter.fill = Color(255, 255, 255);
		node.coplanar.push_back(addToList(splitter));

		node.portal = makePolygonOnPlane(node.plane);
		node.portal = clipPolygon(node.portal, bounds);

		for (const Triangle3& portalTri : triangulate(node.portal, 1, Color(0, 0, 0))) {
			portalTris.push_back(portalTri);
		}
		
		for (int i = 0; i < tris.size(); i++) {

			if (i == splitterIndex) {
				continue;
			}

			
			SplittedTriangle splittedTri = splitTriangle(tris[i], node.plane);

			if (splittedTri.coplanar) {
				Triangle3 coplanarTri = tris[i];
				coplanarTri.fill = Color(255, 255, 255);
				node.coplanar.push_back(addToList(coplanarTri));
			}

			for (const Triangle3& frontTri : splittedTri.front) {
				front.push_back(frontTri);
			}

			for (const Triangle3& backTri : splittedTri.back) {
				back.push_back(backTri);
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

		Color c = generateColor();
		for (int j = 0; j < tris.size(); j++) {
			Triangle3 tri = tris[j];
			tri.fill = changeColor(c, j);
			leaf.indices.push_back(addToList(tri));
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

		if (side == Front) {
			traverse(node.backIndex, cam, world);

			for (int index : node.coplanar) {
				world.addGeometry(list[index]);
			}
			
			traverse(node.frontIndex, cam, world);
		}
		else if (side == Back) {
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

		if (test(node.plane, cam.position) == Back) {
			nodeId = node.backIndex;
		}
		else {
			nodeId = node.frontIndex;
		}
	}

	return 0;
}

// AI
Polygon3D BSP::makePolygonOnPlane(const Plane& plane)
{
	Vec3 center = plane.normal * plane.distance;

	Vec3 temp = fabs(plane.normal.y) < 0.99f ? Vec3(0, 1, 0) : Vec3(1, 0, 0);

	Vec3 axisU = Vec3::normalize(Vec3::cross(temp, plane.normal));
	Vec3 axisV = Vec3::normalize(Vec3::cross(plane.normal, axisU));

	constexpr float size = 1000.0f;

	Polygon3D poly;

	poly.push_back(Vertex3D(center - axisU * size - axisV * size, Vec2(0, 0)));
	poly.push_back(Vertex3D(center + axisU * size - axisV * size, Vec2(1, 0)));
	poly.push_back(Vertex3D(center + axisU * size + axisV * size, Vec2(1, 1)));
	poly.push_back(Vertex3D(center - axisU * size + axisV * size, Vec2(0, 1)));

	return poly;
}

Triangle3DList BSP::triangulate(const Polygon3D& poly, size_t texId, const Color& fill)
{
	Triangle3DList tris;

	if (poly.size() < 3) {
		return tris;
	}

	for (int i = 1; i < poly.size() - 1; i++) {
		Triangle3 tri;

		tri.v0 = poly[0];
		tri.v1 = poly[i];
		tri.v2 = poly[i + 1];
		tri.texId = texId;
		tri.fill = fill;

		tris.push_back(tri);
	}

	return tris;
}

Polygon3D BSP::clipPolygon(const Polygon3D& poly, const PlaneList& bounds)
{
	Polygon3D out = poly;

	for (const Plane& plane : bounds)
	{
		SplittedPolygon splitted = splitAgainstPlane(out, plane);

		if (!splitted.coplanar && splitted.front.size() > 0 && splitted.back.size() > 0) {
			out = splitted.front;
		}
	}

	return out;
}