#pragma once

#include "Triangle.h"
#include "ViewportClipper.h"
#include "Texture.h"
#include "Color.h"
#include "Surface.h"
#include "Debug.h"

struct Span
{
    int y;
    int x1, x2;

    float uz1, uz2;
    float vz1, vz2;
    float iz1, iz2;

    float uzStep;
    float vzStep;
    float izStep;
};

struct EdgeStep2D
{
public:

    int y1, y2;

    float x;
    float uz;
    float vz;
    float iz;

    float xStep;
    float uzStep;
    float vzStep;
    float izStep;

    void step()
    {
        x += xStep;
        uz += uzStep;
        vz += vzStep;
        iz += izStep;
    }
};

class Rasterizer
{
public:
    
    Rasterizer(size_t screenWidth, size_t screenHeight, const Debug& debug)
        : screenWidth(screenWidth), screenHeight(screenHeight), debug(debug)
    {
        zBuffer = static_cast<float*>(std::malloc(screenWidth * screenHeight * sizeof(float)));
    }

    ~Rasterizer()
    {
        std::free(zBuffer);
    }

    void clearBuffers();
    void setFramebuffer(int pitch, uint8_t* pixels);

    void drawLine(const Vec2& vec1, const Vec2& vec2, const Color& color);

    template<bool depthTest, bool screenDoor>
    bool drawTriangle(Triangle2 tri, const CachedSurface& surf, uint8_t mipLevel);

    template<bool depthTest, bool screenDoor>
    bool drawColoredTriangle(Triangle2 tri, const Color& fillColor);

    template<bool depthTest, bool screenDoor>
    int drawPolygon(const Poly2D& poly, const CachedSurface& surf, uint8_t mipLevel);
    
    template<bool depthTest, bool screenDoor>
    int drawColoredPolygon(const Poly2D& poly, const Color& fillColor);

    Color blend(const Color& dstColor, const Color& srcColor, float alpha);
    Color darken(const Color& color);

    void setPixel(int x, int y, const Color& color);
    
private:
    // TODO: framebuffer struct
    size_t screenWidth, screenHeight;
    size_t pitch;

    static const float invTable[16];
    
    
    float* zBuffer;

    uint8_t* framebuffer = nullptr;

    Debug debug;

    EdgeStep2D makeEdge(const Vertex2D& a, const Vertex2D& b);
    Span makeSpan(int y, const EdgeStep2D& a, const EdgeStep2D& b);
    void sortTriangle(Triangle2& triangle);
    
    template<bool depthTest, bool screenDoor>
    void drawSpan(const Span& span, const CachedSurface& surf, uint8_t mipLevel);
    
    template<bool depthTest, bool screenDoor>
    void drawColoredSpan(const Span& span, const Color& fillColor);
};

