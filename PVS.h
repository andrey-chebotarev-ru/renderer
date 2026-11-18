#pragma once

#include <vector>
#include <functional>
#include <string>

#include "common/Common.h"
#include "Vector.h"
#include "Triangle.h"
#include "Math.h"
#include "RenderFaces.h"
#include "Camera.h"
#include "Geometry.h"
#include "BSP.h"
#include "Debug.h"



class PVS
{
public:

	PVS(const Tree& tree);

	void calculate(PVSTable& table);
	void leafFlow(uint32_t leafIdx);

private:

	enum class PortalStatus {
		None,
		Working,
		Done
	};

	struct State
	{
		Portal srcPortal;
		Portal passPortal;

		uint32_t srcPortalIdx;
		uint32_t passPortalIdx;

		IndicesList visitedLeaves;
		BoolList mightSee;
	};

	bool test(const Portal& source, const Portal& target);
	void calculatePortalSee(int portalIdx, BoolList& portalSee);
	void simpleFloodFill(int startLeafIdx, const BoolList& portalSee, BoolList& mightSeeRow);
	void calculateBaseVis();
	
	void portalFlow(uint32_t srcLeafIdx, uint32_t srcPortalIdx, uint32_t threadIdx);
	void clipToSeparators(const Poly3D& source, const Poly3D& pass, Poly3D& target, bool flipClip);

	const Tree& tree;

	MightSeeTable mightSeeTable;
	VisBitsTable visBitsTable;
	std::vector<PortalStatus> portalStatusTable;
};

