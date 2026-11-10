#include "PVS.h"

PVS::PVS(const Tree& tree) :
	tree(tree)
{
}

bool PVS::test(const Portal& source, const Portal& target)
{
	int sourceCount = 0, targetCount = 0;

	// Хоть одна вершина target должна быть перед плоскостью source
	for (const auto& vert : target.poly.verts) {
		if (Geometry::test(vert.pos, source.poly.getPlane()) == Side::Front) {
			targetCount++;
		}
	}

	if (!targetCount) {
		return false;
	}

	// Хоть одна вершина source должна быть позади плоскости target
	for (const auto& vert : source.poly.verts) {
		if (Geometry::test(vert.pos, target.poly.getPlane()) == Side::Back) {
			sourceCount++;
		}
	}

	if (!sourceCount) {
		return false;
	}

	return true;
}

void PVS::calculatePortalSee(int portalIdx, BoolList& portalSee)
{
	const Portal& sourcePortal = tree.portals[portalIdx];

	for (int targetPortalIdx = 0; targetPortalIdx < tree.portals.size(); targetPortalIdx++) {
		const Portal& targetPortal = tree.portals[targetPortalIdx];
		portalSee[targetPortalIdx] = test(sourcePortal, targetPortal);
	}
}

void PVS::simpleFloodFill(int startLeafIdx, const BoolList& portalSee, BoolList& mightSee)
{
	IndicesList stack;
	stack.resize(tree.leaves.size());

	mightSee[startLeafIdx] = true;

	stack.push_back(startLeafIdx);

	while (!stack.empty()) {
		int leafIdx = stack.back();
		stack.pop_back();

		const Leaf& leaf = tree.leaves[leafIdx];

		for (int portalIdx : leaf.portalIdxs) {

			if (!portalSee[portalIdx]) {
				continue;
			}

			int nextLeafIdx = tree.portals[portalIdx].frontLeafIdx;
			
			if (mightSee[nextLeafIdx]) {
				continue;
			}

			mightSee[nextLeafIdx] = true;
			stack.push_back(nextLeafIdx);
		}
	}
}

void PVS::calculateBaseVis(MightSeeTable& table)
{
	BoolList portalSee;
	portalSee.resize(tree.portals.size());
	
	table.resize(tree.portals.size());

	for (int portalIdx = 0; portalIdx < tree.portals.size(); portalIdx++) {
		const Portal& portal = tree.portals[portalIdx];

		std::fill(portalSee.begin(), portalSee.end(), false);

		table[portalIdx].resize(tree.leaves.size());
		std::fill(table[portalIdx].begin(), table[portalIdx].end(), false);

		calculatePortalSee(portalIdx, portalSee);
		simpleFloodFill(portal.frontLeafIdx, portalSee, table[portalIdx]);
	}
}

void PVS::calculate(PVSTable& table)
{
	MightSeeTable mightSeeTable;
	
	calculateBaseVis(mightSeeTable);

	table.resize(tree.leaves.size());

	for (int leafIdx = 0; leafIdx < tree.leaves.size(); leafIdx++) {
		const Leaf& leaf = tree.leaves[leafIdx];

		table[leafIdx].resize(tree.leaves.size());
		std::fill(table[leafIdx].begin(), table[leafIdx].end(), false);
		table[leafIdx][leafIdx] = true;

		for (auto portalIdx : leaf.portalIdxs) {

			for (int i = 0; i < tree.leaves.size(); i++) {
				table[leafIdx][i] = table[leafIdx][i] || mightSeeTable[portalIdx][i];
			}
		}
	}
}