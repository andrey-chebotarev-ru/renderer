#include "PortalRenderer.h"

#include <algorithm>

#include "Math.h"

bool PortalRenderer::projectPortal(ProjectedPortal& out, const Portal& in, const Camera& cam) const
{
	Poly2D renderPoly;
	
	if (!renderer.prepareForRendering(renderPoly, in.poly, cam, false)) {
		return false;
	}

	out.frontLeafIdx = in.frontLeafIdx;
	out.backLeafIdx = in.backLeafIdx;

	// FIX
	out.aabb.min = out.aabb.max = renderPoly.verts[0].pos;

	for (const auto& vert : renderPoly.verts) {
		out.aabb.min.x = std::min(out.aabb.min.x, vert.pos.x);
		out.aabb.min.y = std::min(out.aabb.min.y, vert.pos.y);

		out.aabb.max.x = std::max(out.aabb.max.x, vert.pos.x);
		out.aabb.max.y = std::max(out.aabb.max.y, vert.pos.y);
	}

	return true;
}

void PortalRenderer::render(Camera& cam) const
{
	

	for (int i = 0; i < bsp.getPortalCount(); i++) {
		bsp.getPortal(i).marked = false;
	}

	for (int i = 0; i < bsp.getLeafCount(); i++) {
		bsp.getLeaf(i).vistedAABBs.clear();
		bsp.getLeaf(i).visible = false;
	}

	traverse(bsp.getLeafIdx(cam.position), cam, renderer.getView(), 0);
}

static bool contains(const AABB2D& outer, const AABB2D& inner)
{
	return outer.min.x <= inner.min.x &&
		   outer.min.y <= inner.min.y &&
		   outer.max.x >= inner.max.x &&
		   outer.max.y >= inner.max.y;
}

bool PortalRenderer::isContainedInPrevAABB(int leafIdx, const AABB2D& aabb) const
{
	for (const auto& old : bsp.getLeaf(leafIdx).vistedAABBs) {
		if (contains(old, aabb)) {
			return true;
		}
	}

	bsp.getLeaf(leafIdx).vistedAABBs.push_back(aabb);
	return false;
}

bool crossesNearPlane(const Poly3D& poly, float distance)
{
	bool hasFront = false;
	bool hasBack = false;

	for (const auto& vert : poly.verts) {
		if (vert.pos.z >= distance) {
			hasFront = true;
		}
		else {
			hasBack = true;
		}
	}

	return hasFront && hasBack;
}

void PortalRenderer::traverse(int leafIdx, const Camera& cam, const AABB2D& view, int depth) const
{
	if (depth > 64) {
		return;
	}

	if (isContainedInPrevAABB(leafIdx, view)) {
		return;
	}

	bsp.getLeaf(leafIdx).visible = true;
	
	for (int portalIdx : bsp.getLeaf(leafIdx).portalIdxs) {
		
		AABB2D intersection;
		if (std::abs(Math::distanceToPlane(bsp.getPortal(portalIdx).poly.getPlane(), cam.position)) < 2.0f)
		{
			intersection = view;
		}
		else {
			ProjectedPortal projectedPortal;
			if (!projectPortal(projectedPortal, bsp.getPortal(portalIdx), cam)) {
				continue;
			}


			if (!aabbIntersection(intersection, projectedPortal.aabb, view)) {
				continue;
			}
		}
		
		

		
		
		int otherIdx = (bsp.getPortal(portalIdx).frontLeafIdx == leafIdx ? bsp.getPortal(portalIdx).backLeafIdx : bsp.getPortal(portalIdx).frontLeafIdx);
		
		
		//bsp.getPortal(portalIdx).marked = true;

		traverse(otherIdx, cam, intersection, depth + 1);

		//bsp.getLeaf(otherIdx).visible = true;

		//renderer.drawAABB(projectedPortal.aabb, Color::getDebugColor(projectedPortal.frontLeafIdx + projectedPortal.backLeafIdx));
	}
}

bool PortalRenderer::aabbIntersection(AABB2D& out, const AABB2D& in1, const AABB2D& in2) const
{
	out.min.x = std::max(in1.min.x, in2.min.x);
	out.min.y = std::max(in1.min.y, in2.min.y);

	out.max.x = std::min(in1.max.x, in2.max.x);
	out.max.y = std::min(in1.max.y, in2.max.y);

	return (out.min.x < out.max.x && out.min.y < out.max.y);
}