#pragma once

#include <vector>
#include <functional>
#include <string>

#include "common/Common.h"
#include "common/Vector.h"
#include "common/Face.h"
#include "common/Triangle.h"
#include "common/Math.h"
#include "common/Camera.h"
#include "common/Geometry.h"
#include "common/Bounds.h"

namespace BSP
{
	struct Node
	{
		Plane plane;
		PlaneList bounds;
		FaceList coplanarFaces;
		Bounds::AABB3 aabb;

		int leafIdx;
		int frontIdx;
		int backIdx;
	};

	using NodeList = std::vector<Node>;

	struct Leaf
	{
		IndicesList faceIdxs;
		IndicesList portalIdxs;

		Contents contents = Contents::Empty;

		PlaneList bounds;
	};

	using LeafList = std::vector<Leaf>;

	struct Portal
	{
		Poly3D poly;

		int flippedIdx = -1;

		int nodeIdx = -1;

		int frontNodeIdx = -1;
		int backNodeIdx = -1;

		int frontLeafIdx = -1;
		int backLeafIdx = -1;
	};

	using PortalList = std::vector<Portal>;

	

	struct BSPFace
	{
		Face face;
		int leafIdx;

		BSPFace() {}

		BSPFace(const Face& face, int leafIdx)
			: face(face), leafIdx(leafIdx) {
		}
	};

	using BSPFaceList = std::vector<BSPFace>;

	enum class PortalSide
	{
		Front,
		Back
	};

	struct Splitter
	{
		bool valid;
		Plane plane;
		Contents frontContents;
		Contents backContents;
		int idx;
	};

	

	struct Tree
	{
		NodeList nodes;
		BSPFaceList faces;
		PortalList portals;
		size_t rootNodeIdx;

		size_t getNodeCount() const {
			return nodes.size();
		}

		Node& getNode(size_t idx) {
			return nodes[idx];
		}

		const Node& getNode(size_t idx) const {
			return nodes[idx];
		}

		size_t getLeafCount() const {
			return leaves.size();
		}

		Leaf& getLeaf(size_t idx) {
			return leaves[idx];
		}

		const Leaf& getLeaf(size_t idx) const {
			return leaves[idx];
		}

		size_t getFaceCount() const {
			return faces.size();
		}

		BSPFace& getFace(size_t idx) {
			return faces[idx];
		}

		const BSPFace& getFace(size_t idx) const {
			return faces[idx];
		}

		size_t getPortalCount() const {
			return portals.size();
		}

		Portal& getPortal(size_t idx) {
			return portals[idx];
		}

		const Portal& getPortal(size_t idx) const {
			return portals[idx];
		}

		size_t getRootNodeIdx() const {
			return rootNodeIdx;
		}

		Node& getRootNode() {
			return getNode(rootNodeIdx);
		}

		const Node& getRootNode() const {
			return getNode(rootNodeIdx);
		}

		size_t addNode()
		{
			nodes.push_back(Node());
			return nodes.size() - 1;
		}

		size_t addFace(const Face& face, size_t leafIdx)
		{
			BSPFace bspFace = BSPFace(face, leafIdx);
			faces.push_back(bspFace);
			return faces.size() - 1;
		}

		size_t addLeaf()
		{
			leaves.push_back(Leaf());
			return leaves.size() - 1;
		}

		int addPortal(const Portal& portal)
		{
			portals.push_back(portal);
			return portals.size() - 1;
		}

		void clear()
		{
			nodes.clear();
			leaves.clear();
			faces.clear();
			portals.clear();
		}

		const NodeList& getNodes() const
		{
			return nodes;
		}

		const LeafList& getLeaves() const
		{
			return leaves;
		}

		const BSPFaceList& getFaces() const
		{
			return faces;
		}

		const PortalList& getPortals() const
		{
			return portals;
		}

		void setNodes(const NodeList& nodes)
		{
			this->nodes = nodes;
		}

		void setLeaves(const LeafList& leaves)
		{
			this->leaves = leaves;
		}

		void setFaces(const BSPFaceList& faces)
		{
			this->faces = faces;
		}

		void setPortals(const PortalList& portals)
		{
			this->portals = portals;
		}
		

		/*
		const size_t getLeafIdx(const Vec3& pos) const {
			return BSP::getLeafIdx(*this, pos);
		}
		*/

		/*
		const Leaf& getLeaf(const Vec3& pos) const {
			return getLeaf(getLeafIdx(pos));
		}
		*/

		private:
			LeafList leaves;
	};

	// Private
	namespace
	{
		constexpr size_t maxDepth = 100;

		struct Context
		{
			Context(Tree& tree)
				: tree(tree) {
			}

			Tree& tree;

			size_t nodeIdx;
			PlaneList bounds;

			Contents contents = Contents::Empty;

			size_t depth = 0;
		};

		bool floodFill(Tree& tree, int startLeafIdx);

		Portal createPortal(const Tree& tree, int nodeIdx);
		PortalList carvePortal(const Tree& tree, const Portal& portal);

		Splitter findSplitter(const FaceList& polys);

		void buildRecursive(const FaceList& faces, const Context& ctx);
		void traverse(const Tree& tree, int nodeIndex, const Camera& cam, FaceList& renderFaces, const BoolList& pvsTableRow, bool usePvs);

		PlaneList getPlanes(const Bounds::AABB3& aabb);
		FaceList createWorldBounds(const Bounds::AABB3& aabb, float padding, bool flip = false);

		//void pushFace(const BSPFace& face, int nodeIdx);

		void buildPortalsFromNodeRecursive(Tree& tree, int nodeIdx);
		Portal flipPortal(const Portal& portal);

		void pushPortal(Tree& tree, Portal portal, int frontNodeIdx, int backNodeIdx);
		void refinePortalSide(Tree& tree, const Portal& portal, int splitNodeIdx, int otherNodeIdx, PortalSide side);
		bool clipPortal(const Tree& tree, Portal& portal);
		//void restoreBorderFaces();
	}

	bool compile(Tree& tree, FaceList& polys, const Vec3List& playerStart);
	void render(const Tree& tree, const Camera& cam, FaceList& renderFaces, const BoolList& pvsTableRow);

	PortalList getPortalsInLeaf(const Tree& tree, int leafIdx);
	size_t getLeafIdx(const Tree& tree, const Vec3& pos);
	bool pointInLeaf(const Tree& tree, size_t leafIdx, const Vec3& pos);

	void findTouchedLeaves(const Tree& tree, size_t nodeIdx, const Bounds::AABB3& aabb3, IndicesList& leafIdxs);
}