#pragma once

#include <string>
#include <fstream>

#include "Common.h"
#include "BSP.h"
#include "Texture.h"

class ResourceLoader
{
public:

	ResourceLoader(Tree& tree)
		: tree(tree) {
	}

	bool save(const std::string& path, const StringList& textureNames, const Vec3List& playerStart);

	bool load(const std::string& path, StringList& textureNames, Vec3List& playerStart);
	
private:

	template<typename T>
	void write(const T& value);

	template<typename T>
	T read();

	void writeIndices(const IndicesList& indices);
	IndicesList readIndices();

	void writeString(const std::string& name);
	std::string readString();

	void writeTextureNames(const StringList& names);
	void readTextureNames(StringList& names);

	void writeNode(const Node& node);
	Node readNode();
	
	void writeNodes(const NodeList& nodes);
	NodeList readNodes();

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

	void writePlayerStart(const Vec3List& playerStart);
	Vec3List readPlayerStart();

	void writeTree();
	void readTree();

	Tree& tree;

	std::fstream file;
};

