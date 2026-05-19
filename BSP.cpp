#include "BSP.h"

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
		polygon.front.insert(polygon.front.end(), input.begin(), input.end());
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
	if (tris.size() <= 1) {
		return -1;
	}

	for (int j = 0; j < tris.size(); j++) {
		const Triangle3& tri = tris[j];

		for (int i = 0; i < tris.size(); i++) {
			if (i == j) {
				continue;
			}

			SplittedTriangle splittedTri = splitTriangle(tris[i], tri.getPlane());

			if (splittedTri.back.size() > 0) {
				return j;
			}
		}
	}

	return -1;
}

void BSP::compile(const Triangle3DList& tris)
{
	nodes.clear();
	rootNode = build(tris);
}

void BSP::render(const Camera& cam, World& world)
{
	traverse(rootNode, cam, world);
}

int BSP::build(const Triangle3DList& tris)
{
	if (tris.size() == 0) {
		return -1;
	}

	Node node;

	int splitterIndex = findSplitter(tris);

	if (splitterIndex != -1) {
		Triangle3DList front, back;

		node.isLeaf = false;
		node.plane = tris[splitterIndex].getPlane();
		for (int i = 0; i < tris.size(); i++) {

			if (i == splitterIndex) {
				continue;
			}

			
			SplittedTriangle splittedTri = splitTriangle(tris[i], node.plane);

			for (const Triangle3& frontTri : splittedTri.front) {
				front.push_back(frontTri);
			}

			for (const Triangle3& backTri : splittedTri.back) {
				back.push_back(backTri);
			}
		}

		front.push_back(tris[splitterIndex]);

		if (front.size() != 0) {
			node.frontIndex = build(front);
		}
		else {
			node.frontIndex = -1;
		}

		if (back.size() != 0) {
			node.backIndex = build(back);
		}
		else {
			node.backIndex = -1;
		}

	}
	else {
		node.isLeaf = true;
		node.frontIndex = -1;
		node.backIndex = -1;

		Color c = generateColor();
		for (int j = 0; j < tris.size(); j++) {
			Triangle3 tri = tris[j];
			tri.fill = changeColor(c, j);
			node.tris.push_back(tri);
		}

		//node.tris.insert(node.tris.begin(), tris.begin(), tris.end());
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

	if (node.isLeaf)
	{
		world.addGeometry(node.tris);
	}
	else
	{
		Side side = test(node.plane, cam.position);

		if (side == Front) {
			traverse(node.backIndex, cam, world);
			traverse(node.frontIndex, cam, world);
		}
		else {
			traverse(node.frontIndex, cam, world);
			traverse(node.backIndex, cam, world);
		}

	}
}

int BSP::leaf(const Camera& cam)
{
	int nodeId = rootNode;
	while (true) {
		const Node& node = nodes[nodeId];

		if (node.isLeaf) {
			return nodeId;
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