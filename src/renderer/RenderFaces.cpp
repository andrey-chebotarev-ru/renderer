#include "RenderFaces.h"

RenderFaces::RenderFaces()
{
    faces.reserve(1000);
}

void RenderFaces::addGeometry(const FaceList& faces)
{
    for (const auto& face : faces) {
        this->faces.push_back(face);
    }
}

void RenderFaces::addGeometry(const Face& face)
{
    this->faces.push_back(face);
}

size_t RenderFaces::getSize() const
{
    return faces.size();
}

const Face& RenderFaces::getFace(int idx) const
{
    return faces[idx];
}

Face& RenderFaces::getFace(int idx)
{
    return faces[idx];
}

void RenderFaces::clear()
{
    faces.clear();
}