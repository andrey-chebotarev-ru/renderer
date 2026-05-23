#pragma once

#include <vector>

#include "Triangle.h"
#include "Model.h"

class World
{
public:
    Poly3DList polys;

    World();
    void addGeometry(const Poly3DList& polys);
    void addGeometry(const Poly3D& poly);
    size_t getSize() const;
    const Poly3D& getPolygon(size_t index) const;
    Poly3D& getPolygon(size_t index);
    void clear();
};