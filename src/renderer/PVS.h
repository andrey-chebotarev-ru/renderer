#pragma once

#include <vector>
#include <functional>
#include <string>

#include "common/Common.h"
#include "common/Vector.h"
#include "common/Triangle.h"
#include "common/Math.h"
#include "common/Camera.h"
#include "common/Geometry.h"
#include "common/Debug.h"

#include "RenderFaces.h"
#include "BSP.h"

class PVS
{
public:

	PVS(const BSP::Tree& tree);

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
		BSP::Portal srcPortal;
		BSP::Portal passPortal;

		uint32_t srcPortalIdx;
		uint32_t passPortalIdx;

		IndicesList visitedLeaves;
		BoolList mightSee;
	};

	bool test(const BSP::Portal& source, const BSP::Portal& target);
	void calculatePortalSee(int portalIdx, BoolList& portalSee);
	void simpleFloodFill(int startLeafIdx, const BoolList& portalSee, BoolList& mightSeeRow);
	void calculateBaseVis();
	
	void portalFlow(uint32_t srcLeafIdx, uint32_t srcPortalIdx, uint32_t threadIdx);
	void clipToSeparators(const Poly3D& source, const Poly3D& pass, Poly3D& target, bool flipClip);

	const BSP::Tree& tree;

	MightSeeTable mightSeeTable;
	VisBitsTable visBitsTable;
	std::vector<PortalStatus> portalStatusTable;
};

