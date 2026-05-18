#include "BSP.h"

#include <SDL3/SDL.h>

Side BSP::test(const Plane& plane, const Vec3& vec)
{
	float dist = Math::distanceToPlane(plane, vec);

	if (dist > 0.0f) {
		return Front;
	}
	else if (dist < 0.0f) {
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
	
	// Coplane
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

Node* BSP::compile(const Triangle3DList& tris)
{
	if (tris.size() == 0) {
		return nullptr;
	}

	Triangle3 splitPoly = tris[0];

	Triangle3DList front, back;
	Node* node = new Node();

	for (int i = 1; i < tris.size(); i++) {
		SplittedTriangle splittedTri = splitTriangle(tris[i], splitPoly.getPlane());

		for (const Triangle3& frontTri : splittedTri.front) {
			front.push_back(frontTri);
		}

		for (const Triangle3& backTri : splittedTri.back) {
			back.push_back(backTri);
		}
	}

	if (front.size() != 0) {
		node->front = compile(front);
	}

	if (back.size() != 0) {
		node->back = compile(back);
	}

	node->tri = splitPoly;

	return node;
}

void BSP::traverse(Node* node, const Camera& cam, World& world)
{
	Side side = test(node->tri.getPlane(), cam.position);

	if (side == Front) {

		if (node->back != nullptr) {
			traverse(node->back, cam, world);
		}
		world.addGeometry(node->tri);
		if (node->front != nullptr) {
			traverse(node->front, cam, world);
		}
	}
	else if (side == Back) {
		if (node->front != nullptr) {
			traverse(node->front, cam, world);
		}
		world.addGeometry(node->tri);
		if (node->back != nullptr) {
			traverse(node->back, cam, world);
		}
	}
	else {
		if (node->front != nullptr) {
			traverse(node->front, cam, world);
		}
		if (node->back != nullptr) {
			traverse(node->back, cam, world);
		}
	}
}