#include "Rasterizer.h"

#include <cmath>

const float Rasterizer::invTable[16] = { 1.0f, 1.0f / 2, 1.0f / 3, 1.0f / 4, 1.0f / 5, 1.0f / 6, 1.0f / 7, 1.0f / 8, 1.0f / 9, 1.0f / 10, 1.0f / 11, 1.0f / 12, 1.0f / 13, 1.0f / 14, 1.0f / 15, 1.0f / 16 };
const float Rasterizer::mipScales[4] = { 1.0f, 0.5f, 0.25f, 0.125f };

void Rasterizer::setFramebuffer(int pitch, uint8_t* framebuffer)
{
    this->pitch = pitch;
    this->framebuffer = framebuffer;
}

void Rasterizer::clearBuffers()
{
    memset(framebuffer, 0, screenHeight * pitch);
    memset(zBuffer, 0, screenWidth * screenHeight * sizeof(float));
}

void Rasterizer::sortTriangle(Triangle2& triangle)
{
    if (triangle.v0.pos.y > triangle.v1.pos.y) {
        std::swap(triangle.v0, triangle.v1);
    }

    if (triangle.v0.pos.y > triangle.v2.pos.y) {
        std::swap(triangle.v0, triangle.v2);
    }

    if (triangle.v1.pos.y > triangle.v2.pos.y) {
        std::swap(triangle.v1, triangle.v2);
    }
}

EdgeStep2D Rasterizer::makeEdge(const Vertex2D& a, const Vertex2D& b)
{
    EdgeStep2D edge;

    edge.y1 = std::ceilf(a.pos.y);
    edge.y2 = std::ceilf(b.pos.y);

    float prestep = edge.y1 - a.pos.y;
    float idy = 1.0f / (b.pos.y - a.pos.y);

    edge.xStep = (b.pos.x - a.pos.x) * idy;
    edge.uzStep = (b.uz - a.uz) * idy;
    edge.vzStep = (b.vz - a.vz) * idy;
    edge.izStep = (b.iz - a.iz) * idy;

    edge.x = a.pos.x + prestep * edge.xStep;
    edge.uz = a.uz + prestep * edge.uzStep;
    edge.vz = a.vz + prestep * edge.vzStep;
    edge.iz = a.iz + prestep * edge.izStep;

    return edge;
}

Span __forceinline Rasterizer::makeSpan(int y, const EdgeStep2D& a, const EdgeStep2D& b)
{
    Span span{};

    const EdgeStep2D* left = &a;
    const EdgeStep2D* right = &b;

    if (left->x > right->x) {
        std::swap(left, right);
    }

    span.y = y;

    span.x1 = std::ceilf(left->x);
    span.x2 = std::ceilf(right->x);

    if (span.x1 >= span.x2) {
        return span;
    }

    float prestep = span.x1 - left->x;
    float invDx = 1.0f / (right->x - left->x);

    span.uzStep = (right->uz - left->uz) * invDx;
    span.vzStep = (right->vz - left->vz) * invDx;
    span.izStep = (right->iz - left->iz) * invDx;

    span.uz1 = left->uz + prestep * span.uzStep;
    span.vz1 = left->vz + prestep * span.vzStep;
    span.iz1 = left->iz + prestep * span.izStep;

    span.uz2 = right->uz;
    span.vz2 = right->vz;
    span.iz2 = right->iz;

    return span;
}

void Rasterizer::setPixel(int x, int y, const Color& color)
{
    if (x < 0 || x >= screenWidth || y < 0 || y >= screenHeight) {
        return;
    }

    int pos = (static_cast<unsigned long long>(x) << 2) + y * pitch;
    *(Color*)(framebuffer + pos) = color;
}

template<bool depthTest>
inline void Rasterizer::drawSpan(const Span& span, const CachedSurface& surf, uint8_t mipLevel)
{
    float scale = mipScales[mipLevel];

    if (span.y < 0 || span.y >= screenHeight) {
        return;
    }

    if ((span.x1 < 0 && span.x2 < 0) || (span.x1 >= screenWidth && span.x2 >= screenWidth)) {
        return;
    }

    if (span.x1 == span.x2 || span.x2 <= span.x1) {
        return;
    }

    int x1 = span.x1;
    int x2 = span.x2;

    float uzStart = span.uz1;
    float vzStart = span.vz1;
    float izStart = span.iz1;

    if (x1 < 0)
    {
        uzStart = span.uz1 - x1 * span.uzStep;
        vzStart = span.vz1 - x1 * span.vzStep;
        izStart = span.iz1 - x1 * span.izStep;

        x1 = 0;
    }

    if (x2 > screenWidth) {
        x2 = screenWidth - 1;
    }

    int pos = (static_cast<unsigned long long>(x1) << 2) + span.y * pitch;
    int zindex = x1 + span.y * screenWidth;

    int x = x1;

    int width = surf.widths[mipLevel];
    int height = surf.heights[mipLevel];

    while (x < x2)
    {
        int blockLength = std::min(x2 - x, 16);

        float izEnd = izStart + span.izStep * blockLength;
        float uzEnd = uzStart + span.uzStep * blockLength;
        float vzEnd = vzStart + span.vzStep * blockLength;

        float zStart = 1.0f / izStart;
        float uStart = uzStart * zStart;
        float vStart = vzStart * zStart;

        // Оптимизация
        uStart = (uStart - surf.startU) * scale;
        vStart = (vStart - surf.startV) * scale;

        float zEnd = 1.0f / izEnd;
        float uEnd = uzEnd * zEnd;
        float vEnd = vzEnd * zEnd;

        // Оптимизация
        uEnd = (uEnd - surf.startU) * scale;
        vEnd = (vEnd - surf.startV) * scale;

        float duDx = (uEnd - uStart) * invTable[blockLength - 1];
        float dvDx = (vEnd - vStart) * invTable[blockLength - 1];

        float u = uStart;
        float v = vStart;
        float iz = izStart;

        // Оптимизация
        float uMin = std::min(uStart, uEnd);
        float uMax = std::max(uStart, uEnd);
        
        // Оптимизация
        float vMin = std::min(vStart, vEnd);
        float vMax = std::max(vStart, vEnd);

        bool clamp = (uMin < 0.0f || uMax >= width || vMin < 0.0f || vMax >= height);

        if (clamp) {

            for (int i = 0; i < blockLength; i++) {

                if (!depthTest || zBuffer[zindex] < iz) {

                    int s = static_cast<int>(u);
                    int t = static_cast<int>(v);

                    s = std::clamp(s, 0, width - 1);
                    t = std::clamp(t, 0, height - 1);

                    const Color* c = &surf.pixels[mipLevel][t * width + s];

                    if (!surf.masked || c->a != 0) {
                        *(Color*)(framebuffer + pos) = *c;
                        zBuffer[zindex] = iz;
                    }
                }

                u += duDx;
                v += dvDx;
                iz += span.izStep;
                pos += 4;
                zindex++;
            }                       
        }
        else {

            for (int i = 0; i < blockLength; i++) {

                if (!depthTest || zBuffer[zindex] < iz) {

                    int s = static_cast<int>(u);
                    int t = static_cast<int>(v);

                    const Color* c = &surf.pixels[mipLevel][t * width + s];
                    
                    if (!surf.masked || c->a != 0) {
                        *(Color*)(framebuffer + pos) = *c;
                        zBuffer[zindex] = iz;
                    }
                }

                u += duDx;
                v += dvDx;
                iz += span.izStep;
                pos += 4;
                zindex++;
            }
        }

        x += blockLength;

        uzStart = uzEnd;
        vzStart = vzEnd;
        izStart = izEnd;
    }
}

inline void Rasterizer::drawTranslucentSpan(const Span& span, const CachedSurface& surf, uint8_t mipLevel)
{
    float scale = mipScales[mipLevel];

    if (span.y < 0 || span.y >= screenHeight) {
        return;
    }

    if ((span.x1 < 0 && span.x2 < 0) || (span.x1 >= screenWidth && span.x2 >= screenWidth)) {
        return;
    }

    if (span.x1 == span.x2 || span.x2 <= span.x1) {
        return;
    }

    int x1 = span.x1;
    int x2 = span.x2;

    float uzStart = span.uz1;
    float vzStart = span.vz1;
    float izStart = span.iz1;

    if (x1 < 0)
    {
        uzStart = span.uz1 - x1 * span.uzStep;
        vzStart = span.vz1 - x1 * span.vzStep;
        izStart = span.iz1 - x1 * span.izStep;

        x1 = 0;
    }

    if (x2 > screenWidth) {
        x2 = screenWidth - 1;
    }

    int pos = (static_cast<unsigned long long>(x1) << 2) + span.y * pitch;
    int zindex = x1 + span.y * screenWidth;

    int x = x1;

    int width = surf.widths[mipLevel];
    int height = surf.heights[mipLevel];
    uint32_t alpha = surf.alpha;

    uint32_t* fbPtr = (uint32_t*)(&framebuffer[(x1 << 2) + span.y * pitch]);
    uint32_t* pixels = (uint32_t*)(surf.pixels[mipLevel]);

    while (x < x2)
    {
        int blockLength = std::min(x2 - x, 16);

        float izEnd = izStart + span.izStep * blockLength;
        float uzEnd = uzStart + span.uzStep * blockLength;
        float vzEnd = vzStart + span.vzStep * blockLength;

        float zStart = 1.0f / izStart;
        float uStart = uzStart * zStart;
        float vStart = vzStart * zStart;

        // Оптимизация
        uStart = (uStart - surf.startU) * scale;
        vStart = (vStart - surf.startV) * scale;

        float zEnd = 1.0f / izEnd;
        float uEnd = uzEnd * zEnd;
        float vEnd = vzEnd * zEnd;

        // Оптимизация
        uEnd = (uEnd - surf.startU) * scale;
        vEnd = (vEnd - surf.startV) * scale;

        float duDx = (uEnd - uStart) * invTable[blockLength - 1];
        float dvDx = (vEnd - vStart) * invTable[blockLength - 1];

        float u = uStart;
        float v = vStart;
        float iz = izStart;

        // Оптимизация
        float uMin = std::min(uStart, uEnd);
        float uMax = std::max(uStart, uEnd);

        // Оптимизация
        float vMin = std::min(vStart, vEnd);
        float vMax = std::max(vStart, vEnd);

        bool clamp = (uMin < 0.0f || uMax >= width || vMin < 0.0f || vMax >= height);

        if (clamp) {

            for (int i = 0; i < blockLength; i++) {

                if (zBuffer[zindex] < iz) {

                    int s = static_cast<int>(u);
                    int t = static_cast<int>(v);

                    s = std::clamp(s, 0, width - 1);
                    t = std::clamp(t, 0, height - 1);

                   
                    
                    *fbPtr = blend(pixels[t * width + s], *fbPtr, alpha);
                    
                }

                u += duDx;
                v += dvDx;
                iz += span.izStep;
                pos += 4;
                zindex++;
                fbPtr++;
            }
        }
        else {

            for (int i = 0; i < blockLength; i++) {

                if (zBuffer[zindex] < iz) {

                    int s = static_cast<int>(u);
                    int t = static_cast<int>(v);

                    

                    *fbPtr = blend(pixels[t * width + s], *fbPtr, alpha);
                }

                u += duDx;
                v += dvDx;
                iz += span.izStep;
                pos += 4;
                zindex++;
                fbPtr++;
            }
        }

        x += blockLength;

        uzStart = uzEnd;
        vzStart = vzEnd;
        izStart = izEnd;
    }

}

template<bool depthTest, bool masked, bool clamp>
inline void Rasterizer::drawBlock(int blockLength, int fbOffset, int zbOffset, float uStart, float vStart, float izStart, float uStep, float vStep, float izStep, const CachedSurface& surf, uint8_t mipLevel)
{
    float u = uStart;
    float v = vStart;
    float iz = izStart;

    Color* fbPtr = (Color*)(framebuffer + fbOffset);
    float* zbPtr = (zBuffer + zbOffset);

    for (int i = 0; i < blockLength; i++) {
        
        if (!depthTest || *zbPtr < iz) {
            
            int w = surf.widths[mipLevel];
            int h = surf.heights[mipLevel];
            
            int s = static_cast<int>(u);
            int t = static_cast<int>(v);

            if constexpr (clamp) {
                s = std::clamp(s, 0, w - 1);
                t = std::clamp(t, 0, h - 1);
            }

            const Color* c = &surf.pixels[mipLevel][t * w + s];
            
            if (!masked || c->a != 0) {
                *fbPtr = *c;
                *zbPtr = iz;
            }  
        }
       
        u += uStep;
        v += vStep;
        iz += izStep;
        
        ++fbPtr;
        ++zbPtr;
    }
}

template<bool depthTest>
inline void Rasterizer::drawColoredSpan(const Span& span, const Color& fill)
{
    if (span.y < 0 || span.y >= screenHeight) {
        return;
    }

    if ((span.x1 < 0 && span.x2 < 0) || (span.x1 >= screenWidth && span.x2 >= screenWidth)) {
        return;
    }

    if (span.x1 == span.x2 || span.x2 <= span.x1) {
        return;
    }

    int x1 = span.x1;
    int x2 = span.x2;

    float iz = span.iz1;

    if (x1 < 0)
    {
        iz = span.iz1 - x1 * span.izStep;

        x1 = 0;
    }

    if (x2 >= screenWidth) {
        x2 = screenWidth - 1;
    }

    int pos = (static_cast<unsigned long long>(x1) << 2) + span.y * pitch;
    int zindex = x1 + span.y * screenWidth;

    int x = x1;

    while (x < x2)
    {
        if (!depthTest || zBuffer[zindex] >= iz) {
            *(Color*)(framebuffer + pos) = fill;
            zBuffer[zindex] = iz;
        }
        
        zindex++;
        x++;
        pos += 4;
        iz += span.izStep;
    }
}

void Rasterizer::drawLine(const Vec2& vec1, const Vec2& vec2, const Color& color)
{
    int x1 = ceilf(vec1.x);
    int y1 = ceilf(vec1.y);

    int x2 = ceilf(vec2.x);
    int y2 = ceilf(vec2.y);

    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);

    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;

    int err = dx - dy;

    while (true) {
        setPixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

template<bool depthTest>
bool Rasterizer::drawTriangle(Triangle2 tri, const CachedSurface& surf, uint8_t mipLevel)
{
    sortTriangle(tri);

    if (tri.v0.pos.y == tri.v2.pos.y) {
        return false;
    }

    const Vertex2D& top = tri.v0;
    const Vertex2D& middle = tri.v1;
    const Vertex2D& bottom = tri.v2;

    EdgeStep2D longEdge = makeEdge(top, bottom);

    if (middle.pos.y != top.pos.y) {
        EdgeStep2D topEdge = makeEdge(top, middle);

        for (int y = topEdge.y1; y < topEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, topEdge);

            drawSpan<depthTest>(span, surf, mipLevel);
            
            longEdge.step();
            topEdge.step();
        }
    }

    if (middle.pos.y != bottom.pos.y) {
        EdgeStep2D bottomEdge = makeEdge(middle, bottom);

        for (int y = bottomEdge.y1; y < bottomEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, bottomEdge);

            drawSpan<depthTest>(span, surf, mipLevel);
            
            longEdge.step();
            bottomEdge.step();
        }
    }

 
    return true;
}

template bool Rasterizer::drawColoredTriangle<false>(Triangle2 tri, const Color& fillColor);
template bool Rasterizer::drawColoredTriangle<true>(Triangle2 tri, const Color& fillColor);

template<bool depthTest>
bool Rasterizer::drawColoredTriangle(Triangle2 tri, const Color& fillColor)
{
    sortTriangle(tri);

    if (tri.v0.pos.y == tri.v2.pos.y) {
        return false;
    }

    const Vertex2D& top = tri.v0;
    const Vertex2D& middle = tri.v1;
    const Vertex2D& bottom = tri.v2;

    EdgeStep2D longEdge = makeEdge(top, bottom);

    if (middle.pos.y != top.pos.y) {
        EdgeStep2D topEdge = makeEdge(top, middle);

        for (int y = topEdge.y1; y < topEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, topEdge);
            drawColoredSpan<depthTest>(span, fillColor);
            
            longEdge.step();
            topEdge.step();
        }
    }

    if (middle.pos.y != bottom.pos.y) {
        EdgeStep2D bottomEdge = makeEdge(middle, bottom);

        for (int y = bottomEdge.y1; y < bottomEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, bottomEdge);
            drawColoredSpan<depthTest>(span, fillColor);
            
            longEdge.step();
            bottomEdge.step();
        }
    }

    return true;
}

template int Rasterizer::drawPolygon<false, false>(const Poly2D& poly, const CachedSurface& surf, uint8_t mipLevel);
template int Rasterizer::drawPolygon<false, true>(const Poly2D& poly, const CachedSurface& surf, uint8_t mipLevel);
template int Rasterizer::drawPolygon<true, false>(const Poly2D& poly, const CachedSurface& surf, uint8_t mipLevel);
template int Rasterizer::drawPolygon<true, true>(const Poly2D& poly, const CachedSurface& surf, uint8_t mipLevel);


/*
* Отрисовка выпуклого многоугольника
* 
* Находим верхнюю и нижнюю точки многоугольниа,
* затем берм два ребра которые соединяются с верхней вершиной.
* Определяем и отрисовываем диапазон до окончания ближ. ребра
* и обновляем его. Повторяем до достижении нижней вершины. 
*/
template<bool depthTest, bool translucent>
int Rasterizer::drawPolygon(const Poly2D& poly, const CachedSurface& surf, uint8_t mipLevel)
{
    const auto& verts = poly.verts;
    const int count = verts.size();

    if (count < 3) {
        return 0;
    }

    int topVertIdx = 0;
    int bottomVertIdx = 0;

    // Находим индекс самой верхней и самой нижней вершины
    for (int i = 1; i < count; i++) {

        if (verts[i].pos.y < verts[topVertIdx].pos.y) {
            topVertIdx = i;
        }

        if (verts[i].pos.y > verts[bottomVertIdx].pos.y) {
            bottomVertIdx = i;
        }
    }

    int y = std::ceilf(verts[topVertIdx].pos.y);
    const int maxY = std::ceilf(verts[bottomVertIdx].pos.y);

    if (y >= maxY) {
        return 0;
    }

    // Начинаем от самой верхней вершины,
    // двигаясь по ребрам в противоположные стороны
    int vert1Idx = topVertIdx;
    int vert2Idx = topVertIdx;

    EdgeStep2D edge1, edge2;

    edge1.y2 = y;
    edge2.y2 = y;

    while (y < maxY) {

        // Обход первой стороны
        while (y >= edge1.y2) {
            int nextVertIdx = vert1Idx + 1;

            if (nextVertIdx == count) {
                nextVertIdx = 0;
            }

            if (std::ceilf(verts[nextVertIdx].pos.y) > y) {
                edge1 = makeEdge(verts[vert1Idx], verts[nextVertIdx]);
            }

            vert1Idx = nextVertIdx;
        }

        // Обход противополодной стороны
        while (y >= edge2.y2) {
            int nextVertIdx = vert2Idx - 1;

            if (nextVertIdx < 0) {
                nextVertIdx = count - 1;
            }

            if (std::ceilf(verts[nextVertIdx].pos.y) > y) {
                edge2 = makeEdge(verts[vert2Idx], verts[nextVertIdx]);
            }

            vert2Idx = nextVertIdx;
        }

        const int endY = std::min(edge1.y2, edge2.y2);

        // Отрисовка диапазона
        while (y < endY) {

            Span span = makeSpan(y, edge1, edge2);

            if (translucent) {
                drawTranslucentSpan(span, surf, mipLevel);
            }
            else {
                drawSpan<depthTest>(span, surf, mipLevel);
            }
            
            edge1.step();
            edge2.step();

            y++;
        }
    }

    return 1;
}

template int Rasterizer::drawColoredPolygon<false>(const Poly2D& poly, const Color& fillColor);
template int Rasterizer::drawColoredPolygon<true>(const Poly2D& poly, const Color& fillColor);

template<bool depthTest>
int Rasterizer::drawColoredPolygon(const Poly2D& poly, const Color& fillColor)
{
    Triangle2DList tris = poly.triangulate();
    int count = 0;
    
    for (int i = 0; i < tris.size(); i++) {
        if (drawColoredTriangle<depthTest>(tris[i], fillColor)) {
            count++;
        }
    }

    return count;
}

inline uint32_t Rasterizer::blend(uint32_t src, uint32_t dst, uint32_t alpha)
{
    const uint32_t inv = 256 - alpha;

    uint32_t rb = (
        (src & 0x00FF00FF) * alpha +
        (dst & 0x00FF00FF) * inv
    ) >> 8;
    
    uint32_t ga = (
        ((src >> 8) & 0x00FF00FF) * alpha +
        ((dst >> 8) & 0x00FF00FF) * inv
    );

    return (rb & 0x00FF00FF) | (ga & 0Xff00ff00);
}

Color Rasterizer::darken(const Color& color)
{
    Color c;
    c.val = (color.val >> 2) & 0x003F3F3F;
    c.a = 255;

    return c;
}
