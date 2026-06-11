#pragma once

#include <vector>

#include "Triangle.h"
#include "Model.h"

class RenderFaces
{
public:
    FaceList faces;

    RenderFaces();
    void addGeometry(const FaceList& faces);
    void addGeometry(const Face& face);
    size_t getSize() const;
    const Face& getFace(int idx) const;
    Face& getFace(int idx);
    void clear();
};