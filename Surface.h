#pragma once

#include <cstdint>
#include <algorithm>
#include <vector>

#include "Color.h"
#include "Texture.h"

class CachedSurface
{
public:
    int32_t startU;
    int32_t startV;
    int32_t widths[4];
    int32_t heights[4];
    Color* pixels[4];

    template<bool clamp>
    inline const Color& getSample(float s, float t, uint8_t mipLevel) const
    {
        int w = widths[mipLevel];
        int h = heights[mipLevel];
        
        int u = static_cast<int>(s);
        int v = static_cast<int>(t);

        if constexpr (clamp) {
            u = std::clamp(u, 0, w - 1);
            v = std::clamp(v, 0, h - 1);
        }
        
        return pixels[mipLevel][v * w + u];
    }
};

class SurfaceCache
{
public:
    
    size_t cache(const Face& face, const Texture& tex);
    const CachedSurface& get(size_t idx) const;

    ~SurfaceCache();

private:
    std::vector<Color*> pointers;
    std::vector<CachedSurface> surfaces;
};