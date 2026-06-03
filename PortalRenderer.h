#pragma once

#include <vector>

#include "Renderer.h"
#include "BSP.h"

struct ProjectedPortal
{
	int frontLeafIdx;
	int backLeafIdx;
	AABB2D aabb;
};

using ProjectedPortalList = std::vector<ProjectedPortal>;

class PortalRenderer
{
public:
	PortalRenderer(Renderer& renderer, BSP& bsp)
		: renderer(renderer), bsp(bsp) {

	}

	std::vector<AABB2D> intersections;

	bool projectPortal(ProjectedPortal& out, const Portal& portal, const Camera& cam) const;
	void render(Camera& cam);
	void traverse(int leafIdx, const Camera& cam, const AABB2D& view, int depth);
	bool aabbIntersection(AABB2D& out, const AABB2D& in1, const AABB2D& in2) const;
	bool isContainedInPrevAABB(int leafIdx, const AABB2D& aabb) const;
	void drawAABBs(const Camera& cam) const;

private:
	Renderer& renderer;
	BSP& bsp;
};

