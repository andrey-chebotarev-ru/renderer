#pragma once

#include <vector>
#include <assert.h>


#include "Vector.h"
#include "Math.h"
#include "ViewportClipper.h"
#include "Rasterizer.h"
#include "Model.h"
#include "Camera.h"
#include "Clipper.h"
#include "RenderFaces.h"
#include "Debug.h"
#include "Texture.h"

class Renderer
{
public:
    
    Renderer(size_t screenWidth, size_t screenHeight, float fov, const TextureManager& textureManager, const Debug& debug)
        : screenWidth(screenWidth), screenHeight(screenHeight), rasterizer(screenWidth, screenHeight, debug), viewport(0, screenWidth - 1, 0, screenHeight - 1), textureManager(textureManager), debug(debug)
    {
        screenCenter = Vec2(float(screenWidth) / 2, screenHeight / 2);
        focusDistance = screenCenter.x / tanf(Math::degToRad(fov) / 2);
        aspect = float(screenWidth) / screenHeight;
    }

    int getScreenWidth() const {
        return screenWidth;
    }

    int getScreenHeight() const {
        return screenHeight;
    }

    AABB2D getView() const {
        AABB2D view;
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
    
    Triangle2DList prepareForRendering(const Triangle3& in, const Camera& cam, bool backfaceCulling) const;
    bool prepareForRendering(Poly2D& out, const Poly3D& in, const Camera& cam, bool backfaceCulling) const;

    void setFramebuffer(int pitch, uint8_t* pixels);
    
    int drawScene(const RenderFaces& renderFaces, const ModelList& models, const Camera& cam, bool debug);
    void drawExtras(const FaceList& polys, const Camera& cam);
    void drawLabels(const LabelList& labels, const Camera& cam);
    void drawWireframe(const Poly2D& poly, const Color& color);
    void drawAABB(const AABB2D& aabb, const Color& color);

private:
    static const float nearPlaneDistance, farPlaneDistance;
    int screenWidth, screenHeight;

    Vec2 screenCenter;
    float focusDistance;
    float aspect;
    
    Viewport viewport;

    Clipper clipper;
    ViewportClipper viewportClipper;
    Rasterizer rasterizer;

    Debug debug;
    const TextureManager& textureManager;

    

    
    
    int drawModel(const Model& model, const Camera& cam);
    int drawWorld(const RenderFaces& renderFaces, const Camera& cam, bool debug);

    
};

