#pragma once

#include <string>
#include <fstream>

#include "BSP.h"

class ResourceLoader
{
public:

	ResourceLoader(Tree& tree)
		: tree(tree) {
	}

	bool save(const std::string& path);

	bool load(const std::string& path);

	Vec3 startPos;
	
private:

	template<typename T>
	void write(const T& value);

	template<typename T>
	T read();

	void writeIndices(const IndicesList& indices);
	IndicesList readIndices();

	void writeNode(const Node& node);
	Node readNode();
	
	NodeList readNodes();
	void writeNodes(const NodeList& nodes);

	void writeLeaf(const Leaf& leaf);
	Leaf readLeaf();

	void writeLeaves(const LeafList& leaves);
	LeafList readLeaves();

	void writePoly(const Poly3D& poly);
	Poly3D readPoly();

	void writeFace(const Face& face);
	Face readFace();

	void writeBSPFace(const BSPFace& bspFace);
	BSPFace readBSPFace();

	void writeBSPFaces(const BSPFaceList& bspFaces);
	BSPFaceList readBSPFaces();

	void writePortal(const Portal& portal);
	Portal readPortal();

	void writePortals(const PortalList& portals);
	PortalList readPortals();

	void writeTree();
	void readTree();

	Tree& tree;
	std::fstream file;
};

