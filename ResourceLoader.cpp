#include "ResourceLoader.h"

#include <fstream>



bool ResourceLoader::save(const std::string& path)
{
    file.open(path, std::ios::out | std::ios::binary);

    if (!file) {
        return false;
    }

    writeTree();

    return true;
}

bool ResourceLoader::load(const std::string& path)
{
    file.open(path, std::ios::in | std::ios::binary);

    if (!file) {
        return false;
    }

    readTree();

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

    for (int i = 0; i < count; i++) {
        bspFaces.push_back(readBSPFace());
    }

    return bspFaces;
}

void ResourceLoader::writePortal(const Portal& portal)
{
    writePoly(portal.poly);
    
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

    for (int i = 0; i < count; i++) {
        portals.push_back(readPortal());
    }

    return portals;
}

void ResourceLoader::writeTree()
{
    writeNodes(tree.nodes);
    writeLeaves(tree.leaves);
    writeBSPFaces(tree.faces);
    writePortals(tree.portals);

    write(startPos);
}

void ResourceLoader::readTree()
{
    tree.nodes = readNodes();
    tree.leaves = readLeaves();
    tree.faces = readBSPFaces();
    tree.portals = readPortals();

    startPos = read<Vec3>();
}