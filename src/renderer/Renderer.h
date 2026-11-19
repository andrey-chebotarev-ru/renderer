#pragma once

#include <vector>
#include <assert.h>


#include "common/Vector.h"
#include "common/Math.h"
#include "common/Model.h"
#include "common/Camera.h"
#include "common/Debug.h"
#include "common/Texture.h"
#include "common/Surface.h"
#include "common/Entity.h"
#include "common/Bounds.h"

#include "Clipper.h"
#include "ViewportClipper.h"
#include "RenderFaces.h"
#include "Rasterizer.h"

class Renderer
{
public:
    
    Renderer(size_t screenWidth, size_t screenHeight, float fov, float nearPlaneDistance, float farPlaneDistance, const TextureManager& textureManager, const SurfaceCache& surfaceCache, const Debug& debug)
        : screenWidth(screenWidth), screenHeight(screenHeight), nearPlaneDistance(nearPlaneDistance), farPlaneDistance(farPlaneDistance), rasterizer(screenWidth, screenHeight, debug), viewport(0, screenWidth - 1, 0, screenHeight - 1), textureManager(textureManager), surfaceCache(surfaceCache), debug(debug)
    {
        screenCenter = Vec2(float(screenWidth) / 2, screenHeight / 2);
        focusDistance = screenCenter.x / tanf(Math::degToRad(fov) / 2);
    }

    void setPitchOffset(float pitchOffset) {
        this->pitchOffset = pitchOffset;
    }

    int getScreenWidth() const {
        return screenWidth;
    }

    int getScreenHeight() const {
        return screenHeight;
    }

    Bounds::AABB2 getView() const {
        Bounds::AABB2 view;
        view.min.x = 0;
        view.min.y = 0;
        view.max.x = screenWidth - 1;
        view.max.y = screenHeight - 1;
        return view;
    }

    void transformToWorld(Triangle3& tri, const Transform& tr) const;
    void transformToWorld(Poly3D& poly, const Transform& tr) const;

    Vec3 transformToView(const Vec3& vec, const Camera& cam) const;
    void transformToView(Triangle3& tri, const Camera& cam) const;
    void transformToView(Poly3D& poly, const Camera& cam) const;

    Vec2 project(const Vec3& vec) const;
    Triangle2 project(const Triangle3& tri) const;
    Poly2D project(const Poly3D& poly) const;

    Triangle3DList clip(const Triangle3& in) const;
    Triangle2DList clip(const Triangle2& in) const;

    Poly3D clip(const Poly3D& in) const;
    Poly2D clip(const Poly2D& in) const;
    
    uint8_t getMipLevel(const Triangle3& tri) const;
    uint8_t getMipLevel(const Poly3D& poly) const;

    Triangle2DList prepareForRendering(const Triangle3& in, const Camera& cam, bool backfaceCulling) const;
    bool prepareForRendering(Poly2D& out, const Poly3D& in, const Camera& cam, uint8_t& mipLevel, bool backfaceCulling) const;

    void setFramebuffer(int pitch, uint8_t* pixels);
    
    int drawScene(const RenderFaces& renderFaces, const ModelList& models, const EntityList& entities, const Camera& cam, bool debug);
    void drawLabels(const LabelList& labels, const Camera& cam);
    void drawWireframe(const Poly2D& poly, const Color& color);
    void drawAABB(const Bounds::AABB2& aabb, const Color& color);

    void setPixel(int x, int y, const Color& color);

    int drawWorld(const RenderFaces& renderFaces, const Camera& cam, bool debug);
    int drawEntities(const EntityList& entities, const Camera& cam, bool debug, bool fill);

private:
    const float nearPlaneDistance, farPlaneDistance;
    int screenWidth, screenHeight;

    Vec2 screenCenter;
    float focusDistance;
    
    Viewport viewport;

    Clipper clipper;
    ViewportClipper viewportClipper;
    Rasterizer rasterizer;

    Debug debug;
    const TextureManager& textureManager;
    const SurfaceCache& surfaceCache;

    float pitchOffset = 0.0f;

    

    
    
    int drawModel(const Model& model, const Camera& cam);
    

    
};

