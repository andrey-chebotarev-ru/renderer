#pragma once

#include "ViewportClipper.h"
#include "Texture.h"
#include "Color.h"

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

struct Edge2D
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
    
    Rasterizer(size_t screenWidth, size_t screenHeight)
        : screenWidth(screenWidth), screenHeight(screenHeight)
    {
        zBuffer.resize(screenWidth * screenHeight);
        textures.resize(100);
    }

    void clearBuffers();
    void setFramebuffer(int pitch, uint8_t* pixels);
    void setTexture(const Texture& texture, size_t index);
    Texture getTexture(size_t index) const;

    void drawTriangle(Triangle2 tri, const Texture& tex, bool zTest);
   
    int crossProduct(Vec2 v1, Vec2 v2);
    
    

private:
    // TODO: framebuffer struct
    size_t screenWidth, screenHeight;
    size_t pitch;

    static const float invTable[16];
    
    
    std::vector<float> zBuffer;
    std::vector<Texture> textures;

    uint8_t* framebuffer = nullptr;

    Edge2D makeEdge(const Vertex2D& a, const Vertex2D& b);
    Span makeSpan(int y, const Edge2D& a, const Edge2D& b);
    void sortTriangle(Triangle2& triangle);
    void drawSpan(const Span& span, const Texture& tex, bool zTest);
};

