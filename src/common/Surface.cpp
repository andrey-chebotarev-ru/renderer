#include "Surface.h"

size_t SurfaceCache::cache(const Face& face, const Texture& tex, float amt)
{
    constexpr int BLOCK = 16;
    float minU = face.poly.verts[0].uv.x;
    float minV = face.poly.verts[0].uv.y;
    float maxU = face.poly.verts[0].uv.x;
    float maxV = face.poly.verts[0].uv.y;
    for (const auto& vert : face.poly.verts) {
        minU = std::min(minU, vert.uv.x);
        minV = std::min(minV, vert.uv.y);

        maxU = std::max(maxU, vert.uv.x);
        maxV = std::max(maxV, vert.uv.y);
    }
    int startU = static_cast<int>(std::floor(minU / BLOCK)) * BLOCK;
    int startV = static_cast<int>(std::floor(minV / BLOCK)) * BLOCK;
    int endU = static_cast<int>(std::ceil(maxU / BLOCK)) * BLOCK;
    int endV = static_cast<int>(std::ceil(maxV / BLOCK)) * BLOCK;
    int cacheWidth = endU - startU;
    int cacheHeight = endV - startV;
    int texWidth = tex.getWidth();
    int texHeight = tex.getHeight();
    Color* pixels[4];

    for (int mipLevel = 0; mipLevel < 4; mipLevel++) {
        int cw = cacheWidth >> mipLevel;
        int ch = cacheHeight >> mipLevel;
        int tw = texWidth >> mipLevel;
        int th = texHeight >> mipLevel;
        pixels[mipLevel] = reinterpret_cast<Color*>(std::malloc(cw * ch * sizeof(Color)));

        for (int y = 0; y < ch; y++) {
            int v = (y + (startV >> mipLevel)) % th;

            for (int x = 0; x < cw; x++) {
                int u = (x + (startU >> mipLevel)) % tw;
                if (u < 0) u += tw;
                if (v < 0) v += th;
                pixels[mipLevel][y * cw + x] = tex.pixels[mipLevel][v * tw + u];
            }
        }

        pointers.push_back(pixels[mipLevel]);
    }

    CachedSurface surf;

    surf.name = tex.name;

    surf.startU = startU;
    surf.startV = startV;

    surf.masked = tex.masked;
    surf.alpha = amt;

    for (int mipLevel = 0; mipLevel < 4; mipLevel++) {
        surf.widths[mipLevel] = cacheWidth >> mipLevel;
        surf.heights[mipLevel] = cacheHeight >> mipLevel;
    }
   
    std::memcpy(surf.pixels, pixels, 4 * sizeof(Color*));

    size_t idx = surfaces.size();
    surfaces.push_back(surf);

    return idx;
}

const CachedSurface& SurfaceCache::get(size_t idx) const
{
    return surfaces[idx];
}

SurfaceCache::~SurfaceCache()
{
    for (auto* pointer : pointers) {
        std::free(pointer);
    }

    surfaces.clear();
    pointers.clear();
}