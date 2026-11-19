#pragma once

#include <string>
#include <fstream>

#include "common/Common.h"
#include "common/Texture.h"
#include "common/Entity.h"
#include "common/Map.h"

#include "renderer/BSP.h"

using namespace BSP;

class ResourceLoader
{
public:

	bool save(const std::string& path, const Map& map);

	bool load(const std::string& path, Map& map);
	
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

	void writeLeaf(const BSP::Leaf& leaf);
	BSP::Leaf readLeaf();

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

	void writePortal(const BSP::Portal& portal);
	BSP::Portal readPortal();

	void writePortals(const PortalList& portals);
	PortalList readPortals();

	void writeEntityProperties(const EntityProperties& props);
	void readEntityProperties(EntityProperties& props);

	void writePVSTable(const PVSTable& pvsTable);
	void readPVSTable(PVSTable& pvsTable);

	void writeEntities(const EntityList& entities);
	void readEntities(EntityList& entities);

	void writePlayerStart(const Vec3List& playerStart);
	Vec3List readPlayerStart();


	void writeTree(const BSP::Tree& tree);
	void readTree(BSP::Tree& tree);

	std::fstream file;
};

