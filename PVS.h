#pragma once

#include <vector>
#include <functional>
#include <string>

#include "Common.h"
#include "Vector.h"
#include "Triangle.h"
#include "Math.h"
#include "RenderFaces.h"
#include "Camera.h"
#include "Geometry.h"
#include "BSP.h"



class PVS
{
public:

	PVS(const Tree& tree);

	void calculate(PVSTable& table);

private:

	bool test(const Portal& source, const Portal& target);
	void calculatePortalSee(int portalIdx, BoolList& portalSee);
	void simpleFloodFill(int startLeafIdx, const BoolList& portalSee, BoolList& mightSeeRow);
	void calculateBaseVis(MightSeeTable& table);

	const Tree& tree;
};

