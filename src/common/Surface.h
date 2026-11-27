#pragma once

#include <cstdint>
#include <algorithm>
#include <vector>

#include "Color.h"
#include "Texture.h"
#include "Face.h"

class CachedSurface
{
public:
    std::string name;
    int32_t startU;
    int32_t startV;
    int32_t widths[4];
    int32_t heights[4];
    Color* pixels[4];
    bool masked;
    uint32_t alpha = 255;
};

class SurfaceCache
{
public:
    
    size_t cache(const Face& face, const Texture& tex, float amt);
    const CachedSurface& get(size_t idx) const;

    ~SurfaceCache();

private:
    std::vector<Color*> pointers;
    std::vector<CachedSurface> surfaces;
};