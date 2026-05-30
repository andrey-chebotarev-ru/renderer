#include "World.h"

World::World()
{
    faces.reserve(1000);
}

void World::addGeometry(const FaceList& faces)
{
    for (const auto& face : faces) {
        this->faces.push_back(face);
    }
}

void World::addGeometry(const Face& face)
{
    this->faces.push_back(face);
}

size_t World::getSize() const
{
    return faces.size();
}

const Face& World::getFace(int idx) const
{
    return faces[idx];
}

Face& World::getFace(int idx)
{
    return faces[idx];
}

void World::clear()
{
    faces.clear();
}