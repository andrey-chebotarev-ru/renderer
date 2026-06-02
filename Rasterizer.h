#pragma once

#include "Triangle.h"
#include "ViewportClipper.h"
#include "Texture.h"
#include "Color.h"
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
        zBuffer.resize(screenWidth * screenHeight);
        textures.resize(100);
    }

    void clearBuffers();
    void setFramebuffer(int pitch, uint8_t* pixels);
    void setTexture(const Texture& texture, size_t index);
    Texture getTexture(size_t index) const;

    void drawLine(const Vec2& vec1, const Vec2& vec2, const Color& color);

    template<bool depthTest>
    bool drawTriangle(Triangle2 tri, const Texture& tex);

    template<bool depthTest, bool screenDoor>
    bool drawSolidTriangle(Triangle2 tri, const Color& fillColor);

    template<bool depthTest>
    int drawPolygon(const Poly2D& poly, const Texture& tex);
    
    template<bool depthTest, bool screenDoor>
    int drawSolidPolygon(const Poly2D& poly, const Color& fillColor);

    Color blend(const Color& dstColor, const Color& srcColor, float alpha);
    Color darken(const Color& color);
    
private:
    // TODO: framebuffer struct
    size_t screenWidth, screenHeight;
    size_t pitch;

    static const float invTable[16];
    
    
    std::vector<float> zBuffer;
    std::vector<Texture> textures;

    uint8_t* framebuffer = nullptr;

    Debug debug;

    EdgeStep2D makeEdge(const Vertex2D& a, const Vertex2D& b);
    Span makeSpan(int y, const EdgeStep2D& a, const EdgeStep2D& b);
    void sortTriangle(Triangle2& triangle);
    
    template<bool depthTest>
    void drawSpan(const Span& span, const Texture& tex);
    
    template<bool depthTest, bool screenDoor>
    void drawSolidSpan(const Span& span, const Color& fillColor);
    
    void setPixel(int x, int y, const Color& color);
};

