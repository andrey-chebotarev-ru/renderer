#include "PVS.h"

#include <assert.h>
#include <numeric>
#include <algorithm>

PVS::PVS(const BSP::Tree& tree)
	: tree(tree)
{
}

bool PVS::test(const BSP::Portal& source, const BSP::Portal& target)
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
	const BSP::Portal& sourcePortal = tree.portals[portalIdx];

	for (int targetPortalIdx = 0; targetPortalIdx < tree.portals.size(); targetPortalIdx++) {
		const BSP::Portal& targetPortal = tree.portals[targetPortalIdx];
		portalSee[targetPortalIdx] = test(sourcePortal, targetPortal);
	}
}

void PVS::simpleFloodFill(int startLeafIdx, const BoolList& portalSee, BoolList& mightSee)
{
	IndicesList stack;
	stack.reserve(tree.leaves.size());

	mightSee[startLeafIdx] = true;
	stack.push_back(startLeafIdx);

	while (!stack.empty()) {
		int leafIdx = stack.back();
		stack.pop_back();

		const BSP::Leaf& leaf = tree.leaves[leafIdx];

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

void PVS::calculateBaseVis()
{
	BoolList portalSee;
	portalSee.resize(tree.portals.size());
	
	mightSeeTable.resize(tree.portals.size());

	for (int portalIdx = 0; portalIdx < tree.portals.size(); portalIdx++) {
		const BSP::Portal& portal = tree.portals[portalIdx];

		std::fill(portalSee.begin(), portalSee.end(), false);

		mightSeeTable[portalIdx].resize(tree.leaves.size());
		std::fill(mightSeeTable[portalIdx].begin(), mightSeeTable[portalIdx].end(), false);

		calculatePortalSee(portalIdx, portalSee);
		simpleFloodFill(portal.frontLeafIdx, portalSee, mightSeeTable[portalIdx]);
	}
}

void PVS::portalFlow(uint32_t srcLeafIdx, uint32_t srcPortalIdx, uint32_t threadIdx)
{
	std::vector<State> stack;

	stack.push_back({
		.srcPortal = tree.portals[srcPortalIdx],
		.passPortal = tree.portals[srcPortalIdx],

		.srcPortalIdx = srcPortalIdx,
		.passPortalIdx = srcPortalIdx,

		//.visitedLeaves = {srcLeafIdx},

		.mightSee = mightSeeTable[srcPortalIdx], // Инициализация mightsee

		
		});

	uint64_t processed = 0;
	size_t maxStackSize = 0;

	static uint64_t clippedCount = 0;
	static uint64_t visitedCount = 0;

	while (!stack.empty()) {

		State state = std::move(stack.back());
		stack.pop_back();

		processed++;
		maxStackSize = std::max(maxStackSize, stack.size());

		const BSP::Portal& portal = tree.portals[state.passPortalIdx];
		const BSP::Leaf& leaf = tree.leaves[portal.frontLeafIdx];

		uint32_t currentLeafIdx = portal.frontLeafIdx;

		// Добавляем индекс предыдущего листа
		//state.visitedLeaves.push_back(portal.backLeafIdx);

		// Текущий лист доступен из исходного портала
		visBitsTable[srcPortalIdx][currentLeafIdx] = true;

		assert(tree.portals[srcPortalIdx].backLeafIdx == srcLeafIdx);

		for (auto nextPortalIdx : leaf.portalIdxs) {
			const BSP::Portal& nextPortal = tree.portals[nextPortalIdx];
			uint32_t nextLeafIdx = tree.portals[nextPortalIdx].frontLeafIdx;

			assert(nextPortal.backLeafIdx == currentLeafIdx);

			// Пропускаем портал, ведущий в лист до которого не добраться
			if (!state.mightSee[nextLeafIdx]) {
				continue;
			}

			// Пропускаем портал, ведущий в лист в котором уже побывали
			/*
			if (std::find(state.visitedLeaves.begin(), state.visitedLeaves.end(), nextLeafIdx) != state.visitedLeaves.end()) {

				continue;
			}
			*/

			// Выбираем таблицу видимости для следующего портала
			const auto& testBits = (portalStatusTable[nextPortalIdx] == PortalStatus::Done ? visBitsTable[nextPortalIdx] : mightSeeTable[nextPortalIdx]);

			BoolList nextMightSee(state.mightSee.size());
			bool more = false;

			// Ранняя проверка more
			for (int i = 0; i < nextMightSee.size(); i++) {
				nextMightSee[i] = state.mightSee[i] && testBits[i];

				if (nextMightSee[i] && !visBitsTable[srcPortalIdx][i]) {
					more = true;
				}
			}

			if (!more) {
				continue;
			}

			// Пропускаем портал, ведущий в лист до которого не добраться
			if (!state.mightSee[nextLeafIdx]) {
				continue;
			}

			BSP::Portal targetPortal = nextPortal;
			BSP::Portal passPortal = state.passPortal;
			BSP::Portal srcPortal = state.srcPortal;

			if (state.srcPortalIdx == state.passPortalIdx) {
				targetPortal.poly = Geometry::clip(targetPortal.poly, srcPortal.poly.getPlane(), false);
			}
			else {
				srcPortal.poly = Geometry::clip(srcPortal.poly, targetPortal.poly.getPlane().flip(), false);

				if (!srcPortal.poly.isValid()) {
					clippedCount++;
					continue;
				}

				targetPortal.poly = Geometry::clip(targetPortal.poly, { srcPortal.poly.getPlane(), passPortal.poly.getPlane()}, false);
				
				if (!targetPortal.poly.isValid()) {
					clippedCount++;
					continue;
				}


				clipToSeparators(srcPortal.poly, passPortal.poly, targetPortal.poly, false);

				if (!targetPortal.poly.isValid()) {
					clippedCount++;
					continue;
				}

				clipToSeparators(passPortal.poly, srcPortal.poly, targetPortal.poly, true);

				if (!targetPortal.poly.isValid()) {
					continue;
				}

				clipToSeparators(targetPortal.poly, passPortal.poly, srcPortal.poly, false);

				if (!targetPortal.poly.isValid()) {
					continue;
				}

				clipToSeparators(passPortal.poly, targetPortal.poly, srcPortal.poly, true);

				if (!targetPortal.poly.isValid()) {
					continue;
				}
			}

			if (!targetPortal.poly.isValid() || targetPortal.poly.getArea() < 1.0f) {
				continue;
			}

			//Debug::printf("T{} Pass: {} Target: {}", threadIdx, state.passPortalIdx, nextPortalIdx);

			State nextState = state;

			nextState.srcPortal = std::move(srcPortal);
			nextState.passPortal = std::move(targetPortal);
			nextState.passPortalIdx = nextPortalIdx;
			nextState.mightSee = std::move(nextMightSee);
			//nextState.visitedLeaves.push_back(nextLeafIdx);

			stack.push_back(std::move(nextState));
		}
	}
}

float epsilon = 0.05f;
float minNormalSq = 1e-12f;
void PVS::clipToSeparators(const Poly3D& source, const Poly3D& pass, Poly3D& target, bool flipClip)
{
	const auto& src = source.verts;
	const auto& dst = pass.verts;

	for (size_t i = 0; i < src.size(); i++) {

		size_t next = (i + 1) % src.size();

		Vec3 edge = src[next].pos - src[i].pos;

		for (size_t j = 0; j < dst.size(); j++) {

			Vec3 toPass = dst[j].pos - src[i].pos;

			Vec3 normal = Vec3::cross(edge, toPass);

			float lenSq = normal.lengthSquared();

			if (lenSq < minNormalSq) {
				continue;
			}

			normal /= std::sqrt(lenSq);

			float distance = Vec3::dot(normal, dst[j].pos);

			int sourceSide = 0;

			for (size_t k = 0; k < src.size(); k++) {

				if (k == i || k == next) {
					continue;
				}

				float d = Vec3::dot(normal, src[k].pos) - distance;

				if (d < -epsilon) {
					sourceSide = -1;
					break;
				}

				if (d > epsilon) {
					sourceSide = 1;
					break;
				}

			}

			if (sourceSide == 0) {
				continue;
			}

			if (sourceSide > 0) {
				normal = -normal;
				distance = -distance;
			}

			bool valid = true;
			bool hasFront = false;

			for (size_t k = 0; k < dst.size(); k++) {

				if (k == j) {
					continue;
				}

				float d = Vec3::dot(normal, dst[k].pos) - distance;

				if (d < -epsilon) {
					valid = false;
					break;
				}

				if (d > epsilon) {
					hasFront = true;
				}
			}

			if (!valid || !hasFront) {
				continue;
			}

			if (flipClip) {
				normal = -normal;
				distance = -distance;
			}

			Plane separator(normal, distance);

			target = Geometry::clip(target, separator, false, epsilon);

			if (!target.isValid()) {
				return;
			}
		}
	}
}

void PVS::leafFlow(uint32_t leafIdx)
{
	int threadIdx = 0;
	for (auto portalIdx : tree.leaves[leafIdx].portalIdxs) {
		
		portalFlow(leafIdx, portalIdx, ++threadIdx);
		portalStatusTable[portalIdx] = PortalStatus::Done;
	}
}

void PVS::calculate(PVSTable& table)
{	
	Debug::print("Calculating PVS...");

	calculateBaseVis();

	const size_t portalCount = tree.portals.size();
	const size_t leafCount = tree.leaves.size();

	// Считаем количество потенциально видимых листьев
	IndicesList mightSeeCounts(portalCount);

	for (uint32_t p = 0; p < portalCount; p++) {
		mightSeeCounts[p] = static_cast<uint32_t>(std::count(mightSeeTable[p].begin(), mightSeeTable[p].end(), true));
	}

	IndicesList sortedPortals(portalCount);

	std::iota(sortedPortals.begin(), sortedPortals.end(), 0);

	std::sort(
		sortedPortals.begin(),
		sortedPortals.end(),
		[&](uint32_t a, uint32_t b) {
			return (mightSeeCounts[a] < mightSeeCounts[b]);
		}
	);

	// Инициализируем таблицы
	visBitsTable.resize(portalCount);

	for (auto& bits : visBitsTable) {
		bits.assign(leafCount, false);
	}

	portalStatusTable.assign(portalCount, PortalStatus::None);

	int i = 0;

	// Первый этап: Точная видимость порталов
	for (uint32_t portalIdx : sortedPortals) {

		uint32_t srcLeafIdx = tree.portals[portalIdx].backLeafIdx;

		portalStatusTable[portalIdx] = PortalStatus::Working;

		Debug::printf("BSP::Leaf flow for portal {}/{}", ++i, portalCount);
		portalFlow(srcLeafIdx, portalIdx, 0);

		portalStatusTable[portalIdx] = PortalStatus::Done;
	}

	// Второй этап: собираем PVS листьев
	table.resize(leafCount);

	
	for (uint32_t leafIdx = 0; leafIdx < leafCount; leafIdx++) {

		table[leafIdx].assign(leafCount, false);
		table[leafIdx][leafIdx] = true;

		

		const BSP::Leaf& leaf = tree.leaves[leafIdx];

		for (uint32_t portalIdx : leaf.portalIdxs) {
			const auto& bits = visBitsTable[portalIdx];

			for (uint32_t i = 0; i < leafCount; i++) {
				table[leafIdx][i] = table[leafIdx][i] || bits[i];
			}
		}
	}
}