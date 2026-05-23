#include "World.h"

World::World()
{
    polys.reserve(1000);
}

void World::addGeometry(const Poly3DList& polys)
{
    for (const Poly3D& poly : polys) {
        this->polys.push_back(poly);
    }
}

void World::addGeometry(const Poly3D& poly)
{
    this->polys.push_back(poly);
}

size_t World::getSize() const
{
    return polys.size();
}

const Poly3D& World::getPolygon(size_t index) const
{
    return polys[index];
}

Poly3D& World::getPolygon(size_t index)
{
    return polys[index];
}

void World::clear()
{
    polys.clear();
}
