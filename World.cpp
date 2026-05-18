#include "World.h"

World::World()
{
    tris.reserve(1000);
}

void World::addGeometry(const Triangle3DList& tris)
{
    for (const Triangle3& tri : tris) {
        this->tris.push_back(tri);
    }
}

void World::addGeometry(const Triangle3& tri)
{
    this->tris.push_back(tri);
}

size_t World::getSize() const
{
    return tris.size();
}

const Triangle3& World::getTriangle(size_t index) const
{
    return tris[index];
}

Triangle3& World::getTriangle(size_t index)
{
    return tris[index];
}

void World::clear()
{
    tris.clear();
}
