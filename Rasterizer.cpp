#include "Rasterizer.h"

#include <cmath>

const float Rasterizer::invTable[16] = { 1.0f, 1.0f / 2, 1.0f / 3, 1.0f / 4, 1.0f / 5, 1.0f / 6, 1.0f / 7, 1.0f / 8, 1.0f / 9, 1.0f / 10, 1.0f / 11, 1.0f / 12, 1.0f / 13, 1.0f / 14, 1.0f / 15, 1.0f / 16 };

void Rasterizer::setFramebuffer(int pitch, uint8_t* framebuffer)
{
    this->pitch = pitch;
    this->framebuffer = framebuffer;
}

void Rasterizer::setTexture(const Texture& texture, size_t index)
{
    textures[index] = texture;
}

Texture Rasterizer::getTexture(size_t index) const
{
    return textures[index];
}

void Rasterizer::clearBuffers()
{
    memset(framebuffer, 0, screenHeight * pitch);
    std::fill(zBuffer.begin(), zBuffer.end(), 0.0f);
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

int Rasterizer::crossProduct(Vec2 v1, Vec2 v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

Edge2D Rasterizer::makeEdge(const Vertex2D& a, const Vertex2D& b)
{
    Edge2D edge;

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

Span Rasterizer::makeSpan(int y, const Edge2D& a, const Edge2D& b)
{
    Span span;

    const Edge2D* left = &a;
    const Edge2D* right = &b;

    if (left->x > right->x) {
        std::swap(left, right);
    }

    span.y = y;

    span.x1 = std::ceilf(left->x);
    span.x2 = std::ceilf(right->x);

    float prestep = span.x1 - left->x;
    float idx = 1.0f / (right->x - left->x);

    span.uzStep = (right->uz - left->uz) * idx;
    span.vzStep = (right->vz - left->vz) * idx;
    span.izStep = (right->iz - left->iz) * idx;

    span.uz1 = left->uz + prestep * span.uzStep;
    span.vz1 = left->vz + prestep * span.vzStep;
    span.iz1 = left->iz + prestep * span.izStep;

    span.uz2 = right->uz;
    span.vz2 = right->vz;
    span.iz2 = right->iz;

    return span;
}



void Rasterizer::drawSpan(const Span& span, const Texture& tex, bool zTest)
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

    if (x2 >= screenWidth) {
        x2 = screenWidth - 1;
    }

    int pos = (static_cast<unsigned long long>(x1) << 2) + span.y * pitch;
    int zindex = x1 + span.y * screenWidth;

    int x = x1;

    while (x < x2)
    {
        int blockLength = std::min(x2 - x, 16);

        float izEnd = izStart + span.izStep * blockLength;
        float uzEnd = uzStart + span.uzStep * blockLength;
        float vzEnd = vzStart + span.vzStep * blockLength;

        float zStart = 1.0f / izStart;
        float uStart = uzStart * zStart;
        float vStart = vzStart * zStart;

        float zEnd = 1.0f / izEnd;
        float uEnd = uzEnd * zEnd;
        float vEnd = vzEnd * zEnd;

        float duDx = (uEnd - uStart) * invTable[blockLength - 1];
        float dvDx = (vEnd - vStart) * invTable[blockLength - 1];

        float u = uStart;
        float v = vStart;
        float iz = izStart;

       

        for (int i = 0; i < blockLength; i++) {

            if (zTest) {
                if (zBuffer[zindex] < iz) {
                    *(Color*)(framebuffer + pos) = tex.getSample(u, v);
                    zBuffer[zindex] = iz;
                }
            }
            else {
                *(Color*)(framebuffer + pos) = tex.getSample(u, v);
                zBuffer[zindex] = iz;
            }

            u += duDx;
            v += dvDx;
            iz += span.izStep;


            zindex++;
            pos += 4;
        }


        uzStart = uzEnd;
        vzStart = vzEnd;
        izStart = izEnd;

        x += blockLength;
    }
}

void Rasterizer::drawSolidSpan(const Span& span, bool zTest, bool transparent)
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
        if (zTest) {
            if (zBuffer[zindex] < iz) {
                if (transparent && (((x ^ span.y) & 1) == 0)) {
                    //*(Color*)(framebuffer + pos) = span.fill;
                }
                else {
                    *(Color*)(framebuffer + pos) = span.fill;
                }
                
                zBuffer[zindex] = iz;
            }
        }
        else {
            *(Color*)(framebuffer + pos) = span.fill;
            zBuffer[zindex] = iz;
        }

        
        zindex++;
        x++;
        pos += 4;
        iz += span.izStep;
    }
}

void Rasterizer::drawTriangle(Triangle2 tri, const Texture& tex, bool zTest)
{
    sortTriangle(tri);

    if (tri.v0.pos.y == tri.v2.pos.y) {
        return;
    }

    const Vertex2D& top = tri.v0;
    const Vertex2D& middle = tri.v1;
    const Vertex2D& bottom = tri.v2;

    Edge2D longEdge = makeEdge(top, bottom);

    if (middle.pos.y != top.pos.y) {
        Edge2D topEdge = makeEdge(top, middle);

        for (int y = topEdge.y1; y < topEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, topEdge);
            span.fill = tri.fill;
            drawSpan(span, tex, zTest);
            longEdge.step();
            topEdge.step();
        }
    }

    if (middle.pos.y != bottom.pos.y) {
        Edge2D bottomEdge = makeEdge(middle, bottom);

        for (int y = bottomEdge.y1; y < bottomEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, bottomEdge);
            span.fill = tri.fill;
            drawSpan(span, tex, zTest);
            longEdge.step();
            bottomEdge.step();
        }
    }
}

void Rasterizer::drawSolidTriangle(Triangle2 tri, bool zTest, bool transparent)
{
    sortTriangle(tri);

    if (tri.v0.pos.y == tri.v2.pos.y) {
        return;
    }

    const Vertex2D& top = tri.v0;
    const Vertex2D& middle = tri.v1;
    const Vertex2D& bottom = tri.v2;

    Edge2D longEdge = makeEdge(top, bottom);

    if (middle.pos.y != top.pos.y) {
        Edge2D topEdge = makeEdge(top, middle);

        for (int y = topEdge.y1; y < topEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, topEdge);
            span.fill = tri.fill;
            drawSolidSpan(span, zTest, transparent);
            longEdge.step();
            topEdge.step();
        }
    }

    if (middle.pos.y != bottom.pos.y) {
        Edge2D bottomEdge = makeEdge(middle, bottom);

        for (int y = bottomEdge.y1; y < bottomEdge.y2; y++) {
            Span span = makeSpan(y, longEdge, bottomEdge);
            span.fill = tri.fill;
            drawSolidSpan(span, zTest, transparent);
            longEdge.step();
            bottomEdge.step();
        }
    }
}

Color Rasterizer::blend(const Color& dstColor, const Color& srcColor, float alpha)
{
    int r = int(srcColor.r * alpha + dstColor.r * (1.0f - alpha));
    int g = int(srcColor.r * alpha + dstColor.r * (1.0f - alpha));
    int b = int(srcColor.r * alpha + dstColor.r * (1.0f - alpha));

    return Color(r, g, b);
}

Color Rasterizer::darken(const Color& color)
{
    Color c;
    c.val = (color.val >> 2) & 0x003F3F3F;
    c.a = 255;

    return c;
}
