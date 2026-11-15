#include "ResourceLoader.h"

#include <fstream>

#include "Debug.h"

bool ResourceLoader::save(const std::string& path, const StringList& textureNames, const PVSTable& pvsTable, const Vec3List& playerStart)
{
    file.open(path, std::ios::out | std::ios::binary);

    if (!file) {
        return false;
    }

    Debug::print("Writing texture names...");
    writeTextureNames(textureNames);

    Debug::print("Writing tree...");
    writeTree();

    Debug::print("Writing PVS table...");
    writePVSTable(pvsTable);

    Debug::print("Writing player start positions...");
    writePlayerStart(playerStart);
    
    return true;
}

bool ResourceLoader::load(const std::string& path, StringList& textureNames, PVSTable& pvsTable, Vec3List& playerStart)
{
    file.open(path, std::ios::in | std::ios::binary);

    if (!file) {
        return false;
    }

    Debug::print("Reading texture names...");
    readTextureNames(textureNames);

    Debug::print("Reading tree...");
    readTree();

    Debug::print("Reading PVS table...");
    readPVSTable(pvsTable);
    
    Debug::print("Reading player start positions...");
    playerStart = readPlayerStart();
    
    return true;
}

template<typename T>
void ResourceLoader::write(const T& value)
{
    file.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template<typename T>
T ResourceLoader::read()
{
    T value;
    file.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

void ResourceLoader::writeIndices(const IndicesList& indices)
{
    write(indices.size());
    
    for (int idx : indices) {
        write(idx);
    }
}

IndicesList ResourceLoader::readIndices()
{
    IndicesList indices;

    size_t count = read<size_t>();
    indices.reserve(count);

    for (int i = 0; i < count; i++) {
        indices.push_back(read<int>());
    }

    return indices;
}

void ResourceLoader::writeString(const std::string& name)
{
    size_t length = name.length();
    const char* data = name.data();

    write(length);

    for (int i = 0; i < length; i++) {
        write(data[i]);
    }
}

std::string ResourceLoader::readString()
{
    size_t length = read<size_t>();

    char data[256];

    for (int i = 0; i < length; i++) {
        data[i] = read<char>();
    }

    data[length] = '\0';

    return std::string(data);
}

void ResourceLoader::writeTextureNames(const StringList& names)
{
    Debug::printf("\Texture names: {}", names.size());

    write(names.size());

    for (const auto& name : names) {
        writeString(name);
    }
}

void ResourceLoader::readTextureNames(StringList& names)
{
    size_t count = read<size_t>();
    names.clear();
    names.reserve(count);

    Debug::printf("\tTexture names: {}", count);

    for (int i = 0; i < count; i++) {
        names.push_back(readString());
    }
}

void ResourceLoader::writeNode(const Node& node)
{
    write(node.plane);

    write(node.leafIdx);
    
    write(node.frontIdx);
    write(node.backIdx);
}

Node ResourceLoader::readNode()
{
    Node node;

    node.plane = read<Plane>();

    node.leafIdx = read<int>();
    
    node.frontIdx = read<int>();
    node.backIdx = read<int>();

    return node;
}

void ResourceLoader::writeNodes(const NodeList& nodes)
{
    Debug::printf("\tNodes: {}", nodes.size());

    write(nodes.size());

    for (const auto& node : nodes) {
        writeNode(node);
    }
}

NodeList ResourceLoader::readNodes()
{
    NodeList nodes;

    size_t count = read<size_t>();
    nodes.reserve(count);

    Debug::printf("\tNodes: {}", count);

    for (int i = 0; i < count; i++) {
        nodes.push_back(readNode());
    }

    return nodes;
}

void ResourceLoader::writeLeaf(const Leaf& leaf)
{
    write(leaf.valid);
    write(leaf.outside);
    write(leaf.visible);
    write(leaf.visited);

    writeIndices(leaf.faceIdxs);
    writeIndices(leaf.portalIdxs);
}

Leaf ResourceLoader::readLeaf()
{
    Leaf leaf;

    leaf.valid = read<bool>();
    leaf.outside = read<bool>();
    leaf.visible = read<bool>();
    leaf.visited = read<bool>();

    leaf.faceIdxs = readIndices();
    leaf.portalIdxs = readIndices();

    return leaf;
}

void ResourceLoader::writeLeaves(const LeafList& leaves)
{
    Debug::printf("\tLeaves: {}", leaves.size());

    write(leaves.size());

    for (const auto& leaf : leaves) {
        writeLeaf(leaf);
    }
}

LeafList ResourceLoader::readLeaves()
{
    LeafList leaves;

    size_t count = read<size_t>();
    leaves.reserve(count);

    Debug::printf("Leaves: {}", count);

    for (int i = 0; i < count; i++) {
        leaves.push_back(readLeaf());
    }

    return leaves;
}

void ResourceLoader::writePoly(const Poly3D& poly)
{
    write(poly.getCount());

    for (const auto& vert : poly.verts) {
        write(vert);
    }
}

Poly3D ResourceLoader::readPoly()
{
    Poly3D poly;

    size_t count = read<size_t>();
    
    for (int i = 0; i < count; i++) {
        poly.addVert(read<Vertex3D>());
    }

    return poly;
}

void ResourceLoader::writeFace(const Face& face)
{
    write(face.type);
    writePoly(face.poly);
    write(face.texIdx);
    write(face.debugColor);
}

Face ResourceLoader::readFace()
{
    Face face;

    face.type = read<FaceType>();
    face.poly = readPoly();
    face.texIdx = read<size_t>();
    face.debugColor = read<Color>();

    return face;
}

void ResourceLoader::writeBSPFace(const BSPFace& bspFace)
{
    writeFace(bspFace.face);
    write(bspFace.leafIdx);
}

BSPFace ResourceLoader::readBSPFace()
{
    BSPFace bspFace;

    bspFace.face = readFace();
    bspFace.leafIdx = read<int>();

    return bspFace;
}

void ResourceLoader::writeBSPFaces(const BSPFaceList& bspFaces)
{
    Debug::printf("\tFaces: {}", bspFaces.size());

    write(bspFaces.size());

    for (const auto& bspFace : bspFaces) {
        writeBSPFace(bspFace);
    }
}

BSPFaceList ResourceLoader::readBSPFaces()
{
    BSPFaceList bspFaces;

    size_t count = read<size_t>();
    bspFaces.reserve(count);

    Debug::printf("Faces: {}", count);

    for (int i = 0; i < count; i++) {
        bspFaces.push_back(readBSPFace());
    }

    return bspFaces;
}

void ResourceLoader::writePortal(const Portal& portal)
{
    writePoly(portal.poly);
    
    write(portal.flippedIdx);

    write(portal.nodeIdx);

    write(portal.frontNodeIdx);
    write(portal.backNodeIdx);

    write(portal.frontLeafIdx);
    write(portal.backLeafIdx);

    write(portal.visited);
}

Portal ResourceLoader::readPortal()
{
    Portal portal;

    portal.poly = readPoly();

    portal.flippedIdx = read<int>();

    portal.nodeIdx = read<int>();

    portal.frontNodeIdx = read<int>();
    portal.backNodeIdx = read<int>();

    portal.frontLeafIdx = read<int>();
    portal.backLeafIdx = read<int>();

    portal.visited = read<bool>();

    return portal;
}

void ResourceLoader::writePortals(const PortalList& portals)
{
    Debug::printf("\tPortals: {}", portals.size());

    write(portals.size());

    for (const auto& portal : portals) {
        writePortal(portal);
    }
}

PortalList ResourceLoader::readPortals()
{
    PortalList portals;

    size_t count = read<size_t>();
    portals.reserve(count);

    Debug::printf("Portals: {}", count);

    for (int i = 0; i < count; i++) {
        portals.push_back(readPortal());
    }

    return portals;
}

void ResourceLoader::writePVSTable(const PVSTable& pvsTable)
{
    write(pvsTable.size());

    for (const auto& pvsRow : pvsTable) {

        write(pvsRow.size());

        for (const auto& pvsCell : pvsRow) {
            write(static_cast<uint8_t>(pvsCell));
        }
    }
}

void ResourceLoader::readPVSTable(PVSTable& pvsTable)
{
    size_t rowCount = read<size_t>();

    pvsTable.clear();
    pvsTable.reserve(rowCount);

    for (int j = 0; j < rowCount; j++) {

        size_t cellCount = read<size_t>();

        BoolList pvsRow;
        pvsRow.reserve(cellCount);

        for (int i = 0; i < cellCount; i++) {
            pvsRow.push_back(bool(read<uint8_t>()));
        }

        pvsTable.push_back(pvsRow);
    }
}

void ResourceLoader::writePlayerStart(const Vec3List& playerStart)
{
    Debug::printf("\tStart positions: {}", playerStart.size());

    write(playerStart.size());

    for (const auto& playerStartPos: playerStart) {
        write(playerStartPos);
    }
}

Vec3List ResourceLoader::readPlayerStart()
{
    Vec3List playerStart;

    size_t count = read<size_t>();
    playerStart.reserve(count);

    Debug::printf("Start positions: {}", count);

    for (int i = 0; i < count; i++) {
        playerStart.push_back(read<Vec3>());
    }

    return playerStart;
}

void ResourceLoader::writeTree()
{
    writeNodes(tree.nodes);
    writeLeaves(tree.leaves);
    writeBSPFaces(tree.faces);
    writePortals(tree.portals);
}

void ResourceLoader::readTree()
{
    tree.nodes = readNodes();
    tree.leaves = readLeaves();
    tree.faces = readBSPFaces();
    tree.portals = readPortals();
}