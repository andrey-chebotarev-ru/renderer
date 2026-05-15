#pragma once

#include <vector>

#include "Triangle.h"
#include "Model.h"

class World
{
public:
    Triangle3DList tris;

    World();
    void addGeometry(const Triangle3DList& tris);
    size_t getSize() const;
    const Triangle3& getTriangle(size_t index) const;
    Triangle3& getTriangle(size_t index);
};