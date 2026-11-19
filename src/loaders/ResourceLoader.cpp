#include "ResourceLoader.h"

#include <fstream>

#include "common/Debug.h"

bool ResourceLoader::save(const std::string& path, const Map& map)
{
    file.open(path, std::ios::out | std::ios::binary);

    if (!file) {
        return false;
    }

    Debug::print("Writing texture names...");
    writeTextureNames(map.textureNames);

    Debug::print("Writing tree...");
    writeTree(map.tree);

    Debug::print("Writing PVS table...");
    writePVSTable(map.pvsTable);

    Debug::print("Writing entities...");
    writeEntities(map.entities);

    Debug::print("Writing player start positions...");
    writePlayerStart(map.playerStart);
    
    return true;
}

bool ResourceLoader::load(const std::string& path, Map& map)
{
    file.open(path, std::ios::in | std::ios::binary);

    if (!file) {
        return false;
    }

    Debug::print("Reading texture names...");
    readTextureNames(map.textureNames);

    Debug::print("Reading tree...");
    readTree(map.tree);

    Debug::print("Reading PVS table...");
    readPVSTable(map.pvsTable);
    
    Debug::print("Reading entities...");
    readEntities(map.entities);

    Debug::print("Reading player start positions...");
    map.playerStart = readPlayerStart();
    
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

    std::string result(length, '\0');

    for (int i = 0; i < length; i++) {
        result[i] = read<char>();
    }

    return result;
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

    write(node.aabb);
}

Node ResourceLoader::readNode()
{
    Node node;

    node.plane = read<Plane>();

    node.leafIdx = read<int>();
    
    node.frontIdx = read<int>();
    node.backIdx = read<int>();

    node.aabb = read<Bounds::AABB3>();

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

void ResourceLoader::writeLeaf(const BSP::Leaf& leaf)
{
    write(leaf.valid);
    write(leaf.outside);
    write(leaf.visible);
    write(leaf.visited);

    writeIndices(leaf.faceIdxs);
    writeIndices(leaf.portalIdxs);
}

BSP::Leaf ResourceLoader::readLeaf()
{
    BSP::Leaf leaf;

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

void ResourceLoader::writePortal(const BSP::Portal& portal)
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

BSP::Portal ResourceLoader::readPortal()
{
    BSP::Portal portal;

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

void ResourceLoader::writeEntityProperties(const EntityProperties& props)
{
    write(props.size());

    for (const auto& prop : props) {
        writeString(prop.first);
        writeString(prop.second);
    }
}

void ResourceLoader::readEntityProperties(EntityProperties& props)
{
    size_t propCount = read<size_t>();

    props.clear();
    props.reserve(propCount);

    for (int i = 0; i < propCount; i++) {
        std::string name = readString();
        std::string value = readString();

        props[name] = value;
    }
}

void ResourceLoader::writeEntities(const EntityList& entities)
{
  
    write(entities.size());

    for (const auto& entity : entities) {
        //writeEntityProperties(entity.properties);        
        write(entity.faces.size());

        for (const auto& face : entity.faces) {
            writeFace(face);
        }
    }
}

void ResourceLoader::readEntities(EntityList& entities)
{
    size_t entityCount = read<size_t>();

    Debug::printf("Entities: {}", entityCount);

    entities.clear();
    entities.reserve(entityCount);

    for (int j = 0; j < entityCount; j++) {
        Entity entity;

        //readEntityProperties(entity.properties);

        size_t faceCount = read<size_t>();

        Debug::printf("Faces: {}", faceCount);

        entity.faces.clear();
        entity.faces.reserve(faceCount);

        for (int i = 0; i < faceCount; i++) {
            entity.faces.push_back(readFace());
        }

        entities.push_back(entity);
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

void ResourceLoader::writeTree(const BSP::Tree& tree)
{
    writeNodes(tree.nodes);
    writeLeaves(tree.leaves);
    writeBSPFaces(tree.faces);
    writePortals(tree.portals);
}

void ResourceLoader::readTree(BSP::Tree& tree)
{
    tree.nodes = readNodes();
    tree.leaves = readLeaves();
    tree.faces = readBSPFaces();
    tree.portals = readPortals();
}