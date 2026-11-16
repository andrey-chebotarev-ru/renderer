#pragma once

#include <cstdint>
#include <algorithm>
#include <vector>

#include "Color.h"
#include "Texture.h"

class CachedSurface
{
    const float scales[4] = { 1.0f, 0.5f, 0.25f, 0.125f };

public:
    int32_t startU;
    int32_t startV;
    int32_t width;
    int32_t height;
    Color* pixels[4];

    inline const Color& getSample(float s, float t, uint8_t mipLevel) const
    {
        int w = width >> mipLevel;
        int h = height >> mipLevel;
        int u = static_cast<int>((s - startU) * scales[mipLevel]);
        int v = static_cast<int>((t - startV) * scales[mipLevel]);
        u = std::clamp(u, 0, w - 1);
        v = std::clamp(v, 0, h - 1);
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