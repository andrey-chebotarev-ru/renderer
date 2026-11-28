#pragma once

#include "common/Triangle.h"
#include "common/Texture.h"
#include "common/Color.h"
#include "common/Surface.h"
#include "common/ThreadPool.h"
#include "common/Debug.h"

#include "ViewportClipper.h"

constexpr size_t regionCount = 32;
constexpr size_t threadCount = 8;

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

    inline void step()
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
        frameBuffer = static_cast<uint32_t*>(std::malloc(screenWidth * screenHeight * 4));
    }

    ~Rasterizer()
    {
        std::free(zBuffer);
        std::free(frameBuffer);
    }

    void clearBuffers();
    const uint32_t* getFrameBuffer() const;

    void drawLine(const Vec2& vec1, const Vec2& vec2, const Color& color);

    template<bool depthTest>
    bool drawTriangle(Triangle2 tri, const CachedSurface& surf, uint8_t mipLevel);

    template<bool depthTest>
    bool drawColoredTriangle(Triangle2 tri, const Color& fillColor);

    template<bool depthTest, bool translucent>
    inline int drawPolygon(const Poly2D& poly, const CachedSurface& surf, uint8_t mipLevel);
    
    template<bool depthTest>
    int drawColoredPolygon(const Poly2D& poly, const Color& fillColor);

    static inline uint32_t blend(uint32_t src, uint32_t dst, uint32_t alpha);
    Color darken(const Color& color);

    void setPixel(uint32_t x, uint32_t y, const Color& color);

    size_t rasterize();
    
private:

    struct SpanCommand
    {
        Span span;
        const CachedSurface& surf;
        int y;
        int mipLevel;
        bool translucent;
        bool depthTest;
    };

    std::vector<SpanCommand> regions[regionCount];
    ThreadPool pool = ThreadPool(threadCount);

    // TODO: framebuffer struct
    size_t screenWidth, screenHeight;
    
    static const float invTable[16];
    static const float mipScales[4];
    
    uint32_t* frameBuffer = nullptr;
    float* zBuffer = nullptr;
    
    Debug debug;

    EdgeStep2D makeEdge(const Vertex2D& a, const Vertex2D& b);
    Span makeSpan(int y, const EdgeStep2D& a, const EdgeStep2D& b);
    void sortTriangle(Triangle2& triangle);
    
    template<bool depthTest>
    inline void drawSpan(const Span& span, const CachedSurface& surf, uint8_t mipLevel);
    
    inline void drawTranslucentSpan(const Span& span, const CachedSurface& surf, uint8_t mipLevel);

    template<bool depthTest>
    inline void drawColoredSpan(const Span& span, const Color& fillColor);

    template<bool depthTest, bool masked, bool clamp>
    inline void drawBlock(int blockLength, int fbOffset, int zbOffset, float uStart, float vStart, float izStart, float uStep, float vStep, float izStep, const CachedSurface& surf, uint8_t mipLevel);
};

