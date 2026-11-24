#pragma once

#include <vector>

#include "Triangle.h"

enum class FaceType
{
    Wall,
    Liquid,
    Bounds,
    Debug,
    Entity,
};

enum class Contents
{
    Solid,
    Empty,
};


struct Face
{
    FaceType type = FaceType::Wall;

    Poly3D poly;

    size_t texIdx = 0;
    Color debugColor;
    
    bool canSplit = true;

    size_t surfIdx;

    Face() {};
    Face(const Poly3D& poly, int texIdx, FaceType type = FaceType::Wall, Color debugColor = Color(255, 255, 255))
        : type(type), poly(poly), texIdx(texIdx), debugColor(debugColor) {
    }

    Face(const Face& face, Poly3D& poly)
        : poly(poly)
    {
        copyPropsFrom(face);
    }

    void copyPropsFrom(const Face& face)
    {
        canSplit = face.canSplit;
        texIdx = face.texIdx;
        debugColor = face.debugColor;
        type = face.type;
    }
};

using FaceList = std::vector<Face>;