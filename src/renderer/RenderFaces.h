#pragma once

#include <vector>

#include "common/Triangle.h"
#include "common/Face.h"
#include "common/Model.h"

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